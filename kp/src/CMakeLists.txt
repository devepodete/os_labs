set(EXECUTION_FILENAME tr.out)

cmake_minimum_required(VERSION 3.10)
project(text_editor)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-fsanitize=address -Wall")


add_executable(${EXECUTION_FILENAME} main.cpp)
target_link_libraries(${EXECUTION_FILENAME} ncurses)