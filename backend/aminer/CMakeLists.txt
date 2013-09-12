cmake_minimum_required(VERSION 2.8)
project(aminerserver)

add_subdirectory(act)

add_executable(aminerserver "aminer.hpp" "aminerdata.hpp" "aminerservice.cpp")
target_link_libraries(aminerserver servicebase mgraph indexing)

add_executable(aminerimporter "aminerimporter.cpp" "aminer.hpp" "../base/csvreader.hpp")
target_link_libraries(aminerimporter mgraph)
