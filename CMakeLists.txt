cmake_minimum_required(VERSION 3.10)
project(DatabaseEngine VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Function to check if rebuild needed based on timestamps
function(needs_rebuild TARGET_NAME RESULT)
    set(SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/storage/slotted_page.cpp
        ${CMAKE_SOURCE_DIR}/src/storage/heap_file.cpp
        ${CMAKE_SOURCE_DIR}/src/main.cpp
    )
    
    set(NEED_REBUILD FALSE)
    foreach(SOURCE ${SOURCE_FILES})
        if(NOT EXISTS ${CMAKE_BINARY_DIR}/${TARGET_NAME} OR 
           ${SOURCE} IS_NEWER_THAN ${CMAKE_BINARY_DIR}/${TARGET_NAME})
            set(NEED_REBUILD TRUE)
            break()
        endif()
    endforeach()
    
    if(${NEED_REBUILD})
        execute_process(COMMAND ${CMAKE_COMMAND} --build . --target clean)
    endif()
    
    set(${RESULT} ${NEED_REBUILD} PARENT_SCOPE)
endfunction()

# Check timestamps before build
needs_rebuild(database_engine SHOULD_REBUILD)

# Add components
add_subdirectory(src/storage)

# Main executable
add_executable(database_engine src/main.cpp)
target_link_libraries(database_engine 
    PRIVATE 
        slotted_page
        heap_file
)
