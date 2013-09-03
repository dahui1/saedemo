cmake_minimum_required (VERSION 2.8)
project (weiboserver)

# for ICTCLAS50
if (UNIX)
	add_definitions(-DOS_LINUX)
endif()

add_executable(weiboserver "weibo.hpp" "weibodata.hpp" "weiboservice.cpp" "chinesetokenstream.hpp" "chinesetokenstream.cpp")
target_link_libraries(weiboserver servicebase mgraph indexing "${weiboserver_SOURCE_DIR}/lib/libICTCLAS50.a")

add_executable(weiboimporter "weiboimporter.cpp" "weibo.hpp")
target_link_libraries(weiboimporter mgraph)
