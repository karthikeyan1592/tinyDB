add_library(slotted_page
    src/slotted_page.cpp
)

target_include_directories(slotted_page
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
