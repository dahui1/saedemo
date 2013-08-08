cmake_minimum_required (VERSION 2.8)
project (demoserver)

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS interface.proto)

include_directories("${SAE_SOURCE_DIR}/src/saedb")
include_directories("${PROTOBUF_INCLUDE_DIRS}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

file (GLOB server_SOURCES
    "support_doc_searcher.hpp"
    "aminer.hpp"
    "aminerdata.hpp"
    "expert_searcher.hpp"
    "pminer.hpp"
    "pminerdata.hpp"
    "group_searcher.hpp"
    "weibo.hpp"
    "weibodata.hpp"
    "user_searcher.hpp"
    "search.*"
    "main.cpp"
)

add_executable(server ${server_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(server RpcServer glog gflags ${PROTOBUF_LIBRARY} aminerdata pminerdata weibodata)

file (GLOB aminerimporter_SOURCES "aminerimporter.cpp" "csvreader.*")
add_executable(aminerimporter ${aminerimporter_SOURCES})
target_link_libraries(aminerimporter mgraph)

file (GLOB pminerimporter_SOURCES "pminerimporter.cpp")
add_executable(pminerimporter ${pminerimporter_SOURCES})
target_link_libraries(pminerimporter mgraph)

file (GLOB weiboimporter_SOURCES "weiboimporter.cpp")
add_executable(weiboimporter ${weiboimporter_SOURCES})
target_link_libraries(weiboimporter mgraph)

add_definitions(-DOS_LINUX)
file (GLOB aminerdata_SOURCES "aminerdata.cpp" "aminerdata.hpp")
add_library(aminerdata ${aminerdata_SOURCES})
target_link_libraries(aminerdata mgraph indexing ArnetAnalyzer saedb ${demoserver_BINARY_DIR}/libICTCLAS50.a)

file (GLOB pminerdata_SOURCES "pminerdata.cpp" "pminerdata.hpp")
add_library(pminerdata ${pminerdata_SOURCES})
target_link_libraries(pminerdata mgraph indexing ArnetAnalyzer saedb ${demoserver_BINARY_DIR}/libICTCLAS50.a)

file (GLOB weibodata_SOURCES "weibodata.cpp" "weibodata.hpp")
add_library(weibodata ${weibodata_SOURCES})
target_link_libraries(weibodata mgraph indexing ArnetAnalyzer saedb ${demoserver_BINARY_DIR}/libICTCLAS50.a)
