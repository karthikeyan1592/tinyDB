add_library(heap_file
    heap_file.cpp
)

target_include_directories(heap_file
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(heap_file
    PRIVATE
        slotted_page
)
