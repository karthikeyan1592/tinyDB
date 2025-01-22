#include <iostream>
#include "storage/heap_file.hpp"

struct Movie {
    uint32_t id;
    char title[100];
    float rating;
    uint32_t release;
};

int main() {
    try {
        HeapFile heap_file("movies.db");

        // Insert some movies
        Movie movies[] = {
            {1, "Toy Story", 0.92f, 1995},
            {2, "Black Panther", 0.96f, 2018},
            {3, "Alien", 0.98f, 1979},
            {4, "Star Wars", 0.96f, 1977},
            {5, "The Incredibles", 0.75f, 2004}
        };

        // Store page_id and slot_id for each movie
        std::vector<std::pair<uint32_t, uint16_t>> locations;

        // Insert movies
        for (const auto& movie : movies) {
            uint32_t page_id = heap_file.insertRecord(&movie, sizeof(Movie));
            std::cout << "Inserted movie " << movie.title 
                     << " on page " << page_id << "\n";
        }

        // Print free space map state
        std::cout << "\nFree Space Map after insertions:\n";
        heap_file.printFreeSpaceMap();

        // Sync changes to disk
        heap_file.sync();
        
        // Close the file
        heap_file.close();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
