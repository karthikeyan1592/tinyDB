#ifndef HEAP_FILE_H
#define HEAP_FILE_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include "/root/karthi/2_cmuDBIntro/storage/slotted_page/src/slotted_page.hpp"

class HeapFile {
public:
    // Constants for free-space map
    static constexpr size_t ENTRIES_PER_SECOND_LEVEL = 4; // For demonstration, use 4 as in example
    static constexpr uint8_t MAX_FREE_FRACTION = 8; // 3-bit representation as in example
    
    struct FreeSpaceEntry {
        uint8_t free_fraction; // Value should be divided by MAX_FREE_FRACTION
        
        // Helper method to get actual fraction
        float getFraction() const { 
            return static_cast<float>(free_fraction) / MAX_FREE_FRACTION; 
        }
    };

    // Constructor
    explicit HeapFile(const std::string& filename);

    // Delete copy operations
    HeapFile(const HeapFile&) = delete;
    HeapFile& operator=(const HeapFile&) = delete;
    
    // Delete move operations
    HeapFile(HeapFile&& other) noexcept = delete;
    HeapFile& operator=(HeapFile&& other) noexcept = delete;

    // Default destructor
    ~HeapFile() = default;
    
    // Core operations
    uint32_t insertRecord(const void* record, uint16_t record_size);
    bool deleteRecord(uint32_t page_id, uint16_t slot_id);
    void* getRecord(uint32_t page_id, uint16_t slot_id);
    
    // Free space management
    void updateFreeSpaceMap(uint32_t page_id);
    uint32_t findPageWithSpace(uint16_t required_space);
    void recomputeFreeSpaceMap();
    
    // File operations
    void sync();
    void close();

    // Debugging/Statistics
    void printFreeSpaceMap() const;
    size_t getNumPages() const { return num_pages; }

private:
    std::string filename;
    int file_descriptor;
    size_t num_pages;
    
    // Free space maps
    std::vector<FreeSpaceEntry> free_space_map;
    std::vector<FreeSpaceEntry> second_level_map;
    
    // Cache of recently used pages
    static constexpr size_t PAGE_CACHE_SIZE = 10;
    struct CachedPage {
        uint32_t page_id;
        std::shared_ptr<SlottedPage> page;
        bool is_dirty;
    };
    std::vector<CachedPage> page_cache;

    // Helper methods
    std::shared_ptr<SlottedPage> getPage(uint32_t page_id);
    void flushPage(uint32_t page_id);
    uint32_t allocateNewPage();
    void updateSecondLevelMap(size_t start_idx);
    float calculatePageFreeSpace(const SlottedPage& page);
    void writeFreeSpaceMapToDisk();
    void readFreeSpaceMapFromDisk();
    void initializeMaps();
};

#endif // HEAP_FILE_H