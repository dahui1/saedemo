cmake_minimum_required (VERSION 2.8)
project (pminerserver)

add_executable(pminerserver "pminer.hpp" "pminerdata.hpp" "pminerservice.cpp")
target_link_libraries(pminerserver servicebase mgraph indexing)

add_executable(pminerimporter "pminerimporter.cpp" "pminer.hpp")
target_link_libraries(pminerimporter mgraph)
