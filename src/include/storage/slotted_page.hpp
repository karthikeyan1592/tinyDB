#ifndef SLOTTED_PAGE_H
#define SLOTTED_PAGE_H

#include <cstdint>
#include <memory>
#include <iostream>

class SlottedPage {
public:
    static constexpr std::size_t PAGE_SIZE = 4096;
    
    enum class PageType : uint8_t {
        ROOT,
        INTERNAL,
        LEAF
    };

    struct PageHeader {
        uint32_t id;
        PageType type;
        uint16_t free_start;
        uint16_t free_end;
        uint16_t total_free;
        uint8_t flags;
    };

    struct CellPointer {
        uint16_t cell_location;
        uint16_t cell_size;
    };

    struct PointerList {
        CellPointer* start;
        std::size_t size;
    };

    static constexpr uint8_t CAN_COMPACT = 0x1;

    // Constructor
    SlottedPage(PageType type, uint32_t id);
    
    
    // Delete copy operations
    SlottedPage(const SlottedPage&) = delete;
    SlottedPage& operator=(const SlottedPage&) = delete;
    
    // Allow move operations
    SlottedPage(SlottedPage&& other) noexcept = default;
    SlottedPage& operator=(SlottedPage&& other) noexcept = default;

    // Core operations
    uint16_t addCell(const void* cell, uint16_t cell_size);
    void removeCell(uint16_t idx);
    void* getCell(uint16_t idx);
    void compact();
    
    // I/O operations
    void savePage(int fd) const;
    static std::unique_ptr<SlottedPage> loadPage(int fd, uint32_t page_id);
    
    // Utility methods
    PointerList getPointerList();
    const PageHeader& getHeader() const { return *header(); }
    uint8_t* getData() { return page_data.get(); }
    const uint8_t* getData() const { return page_data.get();
}

private:
    std::unique_ptr<uint8_t[]> page_data;

    // Helper methods
    PageHeader* header() { return reinterpret_cast<PageHeader*>(page_data.get()); }
    const PageHeader* header() const { return reinterpret_cast<const PageHeader*>(page_data.get()); }
    
    static uint16_t cellPointerOffsetToIdx(uint16_t offset) {
        return (offset - sizeof(PageHeader)) / sizeof(CellPointer);
    }
    
    static uint16_t cellPointerIdxToOffset(uint16_t idx) {
        return idx * sizeof(CellPointer) + sizeof(PageHeader);
    }
};

// Output operator for PageType
inline std::ostream& operator<<(std::ostream& os, const SlottedPage::PageType& type) {
    switch (type) {
        case SlottedPage::PageType::LEAF: os << "LEAF"; break;
        case SlottedPage::PageType::INTERNAL: os << "INTERNAL"; break;
        case SlottedPage::PageType::ROOT: os << "ROOT"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}

#endif // SLOTTED_PAGE_H