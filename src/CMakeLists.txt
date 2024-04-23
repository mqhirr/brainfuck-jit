cmake_minimum_required(VERSION 3.20)

file(GLOB SOURCE_FILES
    *.cc
    **/*.cc
)

add_executable(bfjit ${SOURCE_FILES})
target_include_directories(bfjit PRIVATE .)