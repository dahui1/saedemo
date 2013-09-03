cmake_minimum_required (VERSION 2.8)

project (demoserver)

# static output
#set(CMAKE_EXE_LINKER_FLAGS -static -static-libgcc)
#set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
#set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic
#set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
#set(CMAKE_SHARED_LIBRARY_C_FLAGS)         # remove -fPIC
#set(CMAKE_SHARED_LIBRARY_CXX_FLAGS)
#set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)    # remove -rdynamic
#set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

# binary output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${demoserver_BINARY_DIR}")

# try enable c++11
# g++ and clang++ need flags to enable c++11
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" HAS_CPP11)
if (HAS_CPP11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# find saedb library
set(SAEDB_ROOT_PATH "../../saedb" CACHE FILEPATH "root of saedb library")

add_subdirectory (${SAEDB_ROOT_PATH} ${CMAKE_CURRENT_BINARY_DIR}/saedb)
# add includes for sae
include_directories ("${SAEDB_ROOT_PATH}")
# add includes for google-glog
include_directories ("${google-glog_SOURCE_DIR}/src")
# add includes for gflags
include_directories ("${gflags_BINARY_DIR}/include")

set(CMAKE_LIBRARY_PATH "${gflags_BINARY_DIR}/lib" "${google-glog_BINARY_DIR}" ${CMAKE_LIBRARY_PATH})

# include base dir
include_directories ("${demoserver_SOURCE_DIR}/base")
# for generated protobuf files
include_directories ("${demoserver_BINARY_DIR}/base")

add_subdirectory (base)
add_subdirectory (aminer)
add_subdirectory (pminer)
add_subdirectory (weibo)
