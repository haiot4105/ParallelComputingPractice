cmake_minimum_required(VERSION 3.15)
project(Practice_1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

add_library(csv_document src/csvdocument.cpp)
target_include_directories(csv_document PUBLIC include)


add_executable(travelling_salesman src/travelling_salesman.cpp)
add_executable(backpack src/backpack.cpp)

target_link_libraries(travelling_salesman csv_document)
target_link_libraries(backpack csv_document)



install(TARGETS travelling_salesman RUNTIME DESTINATION bin)
install(TARGETS backpack RUNTIME DESTINATION bin)


