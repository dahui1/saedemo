cmake_minimum_required (VERSION 2.8)
project (demoserver)

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS interface.proto)

include_directories("${SAE_SOURCE_DIR}/src/saedb")
include_directories("${PROTOBUF_INCLUDE_DIRS}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

file (GLOB server_SOURCES
    "expert_searcher.*"
    "search.*"
    "aminer.hpp"
    "aminerdata.hpp"
    "main.cpp"
)

add_executable(server ${server_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(server RpcServer glog ${PROTOBUF_LIBRARY} indexing aminerdata)

file (GLOB aminerimporter_SOURCES "aminerimporter.cpp")
add_executable(aminerimporter ${aminerimporter_SOURCES})
target_link_libraries(aminerimporter mgraph)

file (GLOB aminerdata_SOURCES "aminerdata.cpp" "aminerdata.hpp")
add_library(aminerdata ${aminerdata_SOURCES})
target_link_libraries(aminerdata mgraph indexing ArnetAnalyzer saedb)
