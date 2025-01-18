#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <iostream>
#include "heap_file.hpp"

HeapFile::HeapFile(const std::string& fname) : filename(fname) {
    // Open or create the file
    file_descriptor = open(filename.c_str(), O_RDWR | O_CREAT, 0644);
    if (file_descriptor == -1) {
        throw std::runtime_error("Failed to open heap file: " + filename);
    }

    // Get file size and calculate number of pages
    off_t file_size = lseek(file_descriptor, 0, SEEK_END);
    num_pages = file_size / SlottedPage::PAGE_SIZE;
    
    // Initialize maps
    initializeMaps();
    
    // Read existing free space map if file exists
    if (file_size > 0) {
        readFreeSpaceMapFromDisk();
    }
}

void HeapFile::initializeMaps() {
    free_space_map.resize(num_pages, {MAX_FREE_FRACTION});
    second_level_map.resize((num_pages + ENTRIES_PER_SECOND_LEVEL - 1) / ENTRIES_PER_SECOND_LEVEL, {MAX_FREE_FRACTION});
}

uint32_t HeapFile::insertRecord(const void* record, uint16_t record_size) {
    // Find a page with enough space
    uint32_t page_id = findPageWithSpace(record_size);
    if (page_id == num_pages) {
        // No existing page has enough space, allocate new page
        page_id = allocateNewPage();
    }

    auto page = getPage(page_id);
    uint16_t slot_id = page->addCell(record, record_size);
    
    // Mark page as dirty
    auto cache_it = std::find_if(page_cache.begin(), page_cache.end(),
        [page_id](const CachedPage& cached) { return cached.page_id == page_id; });
    if (cache_it != page_cache.end()) {
        cache_it->is_dirty = true;
    }
    
    // Update free space map
    updateFreeSpaceMap(page_id);
    
    return page_id;
}

uint32_t HeapFile::findPageWithSpace(uint16_t required_space) {
    float required_fraction = static_cast<float>(required_space) / SlottedPage::PAGE_SIZE;
    
    // First check second level map
    for (size_t i = 0; i < second_level_map.size(); i++) {
        if (second_level_map[i].getFraction() >= required_fraction) {
            // Check corresponding entries in main map
            size_t start_idx = i * ENTRIES_PER_SECOND_LEVEL;
            size_t end_idx = std::min(start_idx + ENTRIES_PER_SECOND_LEVEL, free_space_map.size());
            
            for (size_t j = start_idx; j < end_idx; j++) {
                if (free_space_map[j].getFraction() >= required_fraction) {
                    auto page = getPage(j);
                    if (calculatePageFreeSpace(*page) >= required_fraction) {
                        return j;
                    }
                    // If actual free space is less, update maps
                    updateFreeSpaceMap(j);
                }
            }
        }
    }
    
    return num_pages; // Indicates no suitable page found
}

void HeapFile::updateFreeSpaceMap(uint32_t page_id) {
    if (page_id >= free_space_map.size()) {
        return;
    }

    auto page = getPage(page_id);
    float free_fraction = calculatePageFreeSpace(*page);
    
    // Update main map
    free_space_map[page_id].free_fraction = 
        std::min(static_cast<uint8_t>(free_fraction * MAX_FREE_FRACTION), MAX_FREE_FRACTION);
    
    // Update second level map
    updateSecondLevelMap(page_id / ENTRIES_PER_SECOND_LEVEL);
}

void HeapFile::updateSecondLevelMap(size_t start_idx) {
    size_t start = start_idx * ENTRIES_PER_SECOND_LEVEL;
    size_t end = std::min(start + ENTRIES_PER_SECOND_LEVEL, free_space_map.size());
    
    uint8_t max_fraction = 0;
    for (size_t i = start; i < end; i++) {
        max_fraction = std::max(max_fraction, free_space_map[i].free_fraction);
    }
    
    second_level_map[start_idx].free_fraction = max_fraction;
}

float HeapFile::calculatePageFreeSpace(const SlottedPage& page) {
    const auto& header = page.getHeader();
    return static_cast<float>(header.total_free) / SlottedPage::PAGE_SIZE;
}

void HeapFile::recomputeFreeSpaceMap() {
    for (size_t i = 0; i < num_pages; i++) {
        updateFreeSpaceMap(i);
    }
    writeFreeSpaceMapToDisk();
}

uint32_t HeapFile::allocateNewPage() {
    uint32_t new_page_id = num_pages++;
    auto new_page = std::make_unique<SlottedPage>(SlottedPage::PageType::LEAF, new_page_id);
    
    // Extend free space maps
    free_space_map.push_back({MAX_FREE_FRACTION});
    if (new_page_id % ENTRIES_PER_SECOND_LEVEL == 0) {
        second_level_map.push_back({MAX_FREE_FRACTION});
    }
    
    // Save the new page
    new_page->savePage(file_descriptor);
    
    return new_page_id;
}

std::shared_ptr<SlottedPage> HeapFile::getPage(uint32_t page_id) {
    // Check cache first
    auto cache_it = std::find_if(page_cache.begin(), page_cache.end(),
        [page_id](const CachedPage& cached) { return cached.page_id == page_id; });
    
    if (cache_it != page_cache.end()) {
        return cache_it->page;
    }
    
    // Create new page
    auto page = std::make_shared<SlottedPage>(SlottedPage::PageType::LEAF, page_id);
    
    // Load data if page exists
    if (page_id < num_pages) {
        // Load existing page data
        lseek(file_descriptor, page_id * SlottedPage::PAGE_SIZE, SEEK_SET);
        read(file_descriptor, page->getData(), SlottedPage::PAGE_SIZE);
    }
    
    // Cache management
    if (page_cache.size() >= PAGE_CACHE_SIZE) {
        if (page_cache.front().is_dirty) {
            flushPage(page_cache.front().page_id);
        }
        page_cache.erase(page_cache.begin());
    }
    
    page_cache.push_back({page_id, page, false});
    return page;
}

void HeapFile::flushPage(uint32_t page_id) {
    auto cache_it = std::find_if(page_cache.begin(), page_cache.end(),
        [page_id](const CachedPage& cached) { return cached.page_id == page_id; });
    
    if (cache_it != page_cache.end() && cache_it->is_dirty) {
        cache_it->page->savePage(file_descriptor);
        cache_it->is_dirty = false;
    }
}

void HeapFile::sync() {
    // Flush all dirty pages
    for (auto& cached : page_cache) {
        if (cached.is_dirty) {
            flushPage(cached.page_id);
        }
    }
    
    // Write free space map
    writeFreeSpaceMapToDisk();
    
    // Sync file to disk
    fsync(file_descriptor);
}

void HeapFile::close() {
    sync();
    ::close(file_descriptor);
}

void HeapFile::writeFreeSpaceMapToDisk() {
    // Write maps at the end of the file
    off_t maps_offset = num_pages * SlottedPage::PAGE_SIZE;
    lseek(file_descriptor, maps_offset, SEEK_SET);
    
    write(file_descriptor, free_space_map.data(), free_space_map.size() * sizeof(FreeSpaceEntry));
    write(file_descriptor, second_level_map.data(), second_level_map.size() * sizeof(FreeSpaceEntry));
}

void HeapFile::readFreeSpaceMapFromDisk() {
    off_t maps_offset = num_pages * SlottedPage::PAGE_SIZE;
    lseek(file_descriptor, maps_offset, SEEK_SET);
    
    read(file_descriptor, free_space_map.data(), free_space_map.size() * sizeof(FreeSpaceEntry));
    read(file_descriptor, second_level_map.data(), second_level_map.size() * sizeof(FreeSpaceEntry));
}

void HeapFile::printFreeSpaceMap() const {
    std::cout << "Main Free Space Map:\n";
    for (size_t i = 0; i < free_space_map.size(); i++) {
        std::cout << "Block " << i << ": " << static_cast<int>(free_space_map[i].free_fraction) 
                 << "/" << static_cast<int>(MAX_FREE_FRACTION) << "\n";
    }
    
    std::cout << "\nSecond Level Map:\n";
    for (size_t i = 0; i < second_level_map.size(); i++) {
        std::cout << "Entry " << i << ": " << static_cast<int>(second_level_map[i].free_fraction)
                 << "/" << static_cast<int>(MAX_FREE_FRACTION) << "\n";
    }
}