cmake_minimum_required(VERSION 3.10)
project(DatabaseEngine VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(storage)
add_subdirectory(heap_file)

# Create main executable
add_executable(database_engine main.cpp)

# Link against our libraries
target_link_libraries(database_engine 
    PRIVATE 
        slotted_page
        heap_file
)