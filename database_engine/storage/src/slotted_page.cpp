#include <cassert>
#include <cstring>
#include <sys/mman.h>
#include "slotted_page.hpp"
#include <unistd.h> // Include for lseek, write, fsync, read
#include <fcntl.h>  // Include for open

SlottedPage::SlottedPage(PageType type, uint32_t id) 
    : page_data(std::make_unique<uint8_t[]>(PAGE_SIZE)) {
    auto* hdr = header();
    hdr->id = id;
    hdr->type = type;
    hdr->free_start = sizeof(PageHeader);
    hdr->free_end = PAGE_SIZE - 1;
    hdr->total_free = hdr->free_end - hdr->free_start;
    hdr->flags = 0;
}

uint16_t SlottedPage::addCell(const void* cell, uint16_t cell_size) {
    auto* hdr = header();
    assert(hdr->total_free >= cell_size + sizeof(CellPointer));

    CellPointer cell_pointer;
    cell_pointer.cell_location = hdr->free_end - cell_size;
    cell_pointer.cell_size = cell_size;

    // Add the cell to the page
    std::memcpy(page_data.get() + cell_pointer.cell_location, cell, cell_size);

    // Add cell pointer to the page
    uint16_t pointer_offset = hdr->free_start;
    std::memcpy(page_data.get() + pointer_offset, &cell_pointer, sizeof(CellPointer));

    // Update the header
    hdr->free_end -= cell_size;
    hdr->free_start += sizeof(CellPointer);
    hdr->total_free = hdr->free_end - hdr->free_start;

    return cellPointerOffsetToIdx(pointer_offset);
}

void SlottedPage::removeCell(uint16_t idx) {
    uint16_t pointer_offset = cellPointerIdxToOffset(idx);
    auto* hdr = header();
    hdr->flags |= CAN_COMPACT;
    reinterpret_cast<CellPointer*>(page_data.get() + pointer_offset)->cell_location = 0;
}

void* SlottedPage::getCell(uint16_t idx) {
    uint16_t pointer_offset = cellPointerIdxToOffset(idx);
    uint16_t cell_location = reinterpret_cast<CellPointer*>(page_data.get() + pointer_offset)->cell_location;

    if (cell_location == 0) {
        return nullptr;
    }

    return page_data.get() + cell_location;
}

void SlottedPage::compact() {
    auto* hdr = header();
    PointerList plist = getPointerList();

    if (!(hdr->flags & CAN_COMPACT))
        return;

    auto temp_page = std::make_unique<SlottedPage>(PageType::ROOT, 0);
    CellPointer* cur_pointer;
    
    for (size_t i = 0; i < plist.size; i++) {
        cur_pointer = plist.start + i;
        if (cur_pointer->cell_location != 0) {
            temp_page->addCell(page_data.get() + cur_pointer->cell_location, cur_pointer->cell_size);
        }
    }

    // Copy the compacted data back
    const auto* temp_hdr = temp_page->header();
    hdr->free_start = temp_hdr->free_start;
    hdr->free_end = temp_hdr->free_end;
    hdr->total_free = temp_hdr->total_free;
    hdr->flags &= ~CAN_COMPACT;
    std::memcpy(
        page_data.get() + sizeof(PageHeader),
        temp_page->getData() + sizeof(PageHeader),
        PAGE_SIZE - sizeof(PageHeader)
    );
}
/*
void SlottedPage::savePage(int fd) const {
    void* mapped = mmap(nullptr, PAGE_SIZE, PROT_WRITE, MAP_SHARED, fd, header()->id * PAGE_SIZE);
    if (mapped == MAP_FAILED) {
        throw std::runtime_error("Failed to mmap file for writing");
    }

    std::memcpy(mapped, page_data.get(), PAGE_SIZE);

    if (munmap(mapped, PAGE_SIZE) == -1) {
        throw std::runtime_error("Failed to unmap file after writing");
    }
}

std::unique_ptr<SlottedPage> SlottedPage::loadPage(int fd, uint32_t page_id) {
    void* mapped = mmap(nullptr, PAGE_SIZE, PROT_READ, MAP_SHARED, fd, page_id * PAGE_SIZE);
    if (mapped == MAP_FAILED) {
        throw std::runtime_error("Failed to mmap file for reading");
    }

    auto page = std::make_unique<SlottedPage>(PageType::ROOT, page_id);
    std::memcpy(page->getData(), mapped, PAGE_SIZE);

    if (munmap(mapped, PAGE_SIZE) == -1) {
        throw std::runtime_error("Failed to unmap file after reading");
    }

    return page;
}*/
void SlottedPage::savePage(int fd) const {
    const auto* header = reinterpret_cast<const PageHeader*>(page_data.get());
    off_t offset = header->id * PAGE_SIZE;

    if (lseek(fd, offset, SEEK_SET) == -1) {
        throw std::runtime_error("Failed to seek to the correct position in the file");
    }

    if (write(fd, page_data.get(), PAGE_SIZE) != PAGE_SIZE) {
        throw std::runtime_error("Failed to write the page to the file");
    }
}

std::unique_ptr<SlottedPage> SlottedPage::loadPage(int fd, uint32_t page_id) {
    auto page = std::make_unique<SlottedPage>(PageType::ROOT, page_id);
    off_t offset = page_id * PAGE_SIZE;

    if (lseek(fd, offset, SEEK_SET) == -1) {
        throw std::runtime_error("Failed to seek to the correct position in the file");
    }

    if (read(fd, page->getData(), PAGE_SIZE) != PAGE_SIZE) {
        throw std::runtime_error("Failed to read the page from the file");
    }

    return page;
}

SlottedPage::PointerList SlottedPage::getPointerList() {
    PointerList list;
    list.start = reinterpret_cast<CellPointer*>(page_data.get() + sizeof(PageHeader));
    list.size = (header()->free_start - sizeof(PageHeader)) / sizeof(CellPointer);
    return list;
}