# [src/storage/CMakeLists.txt]
add_library(slotted_page slotted_page.cpp)
add_library(heap_file heap_file.cpp)

# Add include path for both targets
target_include_directories(slotted_page PUBLIC ${CMAKE_SOURCE_DIR}/src/include)
target_include_directories(heap_file PUBLIC ${CMAKE_SOURCE_DIR}/src/include)

# Link dependencies if any
# target_link_libraries(slotted_page ...)
target_link_libraries(heap_file PRIVATE slotted_page)