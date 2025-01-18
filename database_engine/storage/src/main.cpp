#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <cassert>
#include <cstring>

#include "slotted_page.hpp"

struct Movie {
    uint32_t id;
    char title[100];
    float rating;
    uint32_t release;
};

void display_header(const SlottedPage& page) {
    const auto& header = page.getHeader();
    std::cout << "Page: id=" << header.id
              << ", type=" << header.type
              << ", free_start=" << header.free_start
              << ", free_end=" << header.free_end
              << ", total_free=" << header.total_free
              << ", flags=" << static_cast<int>(header.flags) << '\n';
}

void display_movie(const Movie& movie) {
    std::cout << "Movie: id=" << movie.id
              << ", title=" << movie.title
              << ", rating=" << movie.rating
              << ", release=" << movie.release << '\n';
}

int main() {
    const std::string filepath = "movies.db";
    
    // Create and populate page
    auto page = std::make_unique<SlottedPage>(SlottedPage::PageType::LEAF, 0);
    display_header(*page);

    Movie movies[] = {
        {1, "Toy Story", 0.92f, 1995},
        {2, "Black Panther", 0.96f, 2018},
        {3, "Alien", 0.98f, 1979},
        {4, "Star Wars", 0.96f, 1977},
        {5, "The Incredibles", 0.75f, 2004}
    };

    std::vector<uint16_t> cells;
    for (const auto& movie : movies) {
        cells.push_back(page->addCell(&movie, sizeof(Movie)));
    }

    display_header(*page);

    // Save and load page
    int fd = open(filepath.c_str(), O_RDWR | O_CREAT, 0644);
    page->savePage(fd);
    close(fd);

    fd = open(filepath.c_str(), O_RDWR);
    auto loaded_page = SlottedPage::loadPage(fd, 0);
    close(fd);

    // Display all movies
    for (auto cell_idx : cells) {
        if (auto* movie = static_cast<Movie*>(loaded_page->getCell(cell_idx))) {
            display_movie(*movie);
        }
    }

    // Remove some cells and compact
    std::cout << "\nRemoving cells...\n";
    loaded_page->removeCell(cells[2]); // Remove Alien
    loaded_page->removeCell(cells[4]); // Remove The Incredibles

    std::cout << "Before compact:\n";
    display_header(*loaded_page);

    loaded_page->compact();

    std::cout << "After compact:\n";
    display_header(*loaded_page);

    // Display remaining movies
    auto plist = loaded_page->getPointerList();
    std::cout << "Pointer list size: " << plist.size << '\n';
    for (size_t i = 0; i < plist.size; ++i) {
        const auto& cur = plist.start[i];
        if (cur.cell_location != 0) {
            display_movie(*reinterpret_cast<Movie*>(loaded_page->getData() + cur.cell_location));
        }
    }

    return 0;
}