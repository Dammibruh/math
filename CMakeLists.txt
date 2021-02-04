cmake_minimum_required(VERSION 3.5)
project(ami LANGUAGES CXX)
option(ADD_EXAMPLES "Compile the examples" ON)
option(REPL "Compile the Repl" ON)
set(CMAKE_CXX_STANDARD 17)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

if(ADD_EXAMPLES)
    add_subdirectory(examples)
endif()
if(REPL)
    add_subdirectory(repl)
endif()
