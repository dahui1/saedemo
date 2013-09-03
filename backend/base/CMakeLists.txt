cmake_minimum_required (VERSION 2.8)
project (demoserverbase)

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS interface.proto)

add_library(servicebase "main.cpp" "event.cpp" "event.hpp" "searchservice.hpp" ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(servicebase zrpc gflags google-glog ${PROTOBUF_LIBRARY})
