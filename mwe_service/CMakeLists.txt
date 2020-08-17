cmake_minimum_required(VERSION 3.7)

# The version number.
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)

# Set paths
project(mwe_service CXX)

# use C++11
set (CMAKE_CXX_STANDARD 11)

# Update number of threads
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DCPPHTTPLIB_THREAD_POOL_COUNT=40")

# include directories
include_directories(${CMAKE_CURRENT_LIST_DIR}/src)

# write the exe here
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin/")

# get the src files
file(GLOB MWE_SERVICE_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

add_executable(mwe_service ${MWE_SERVICE_SRC})

target_link_libraries(mwe_service pthread)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
include (CPack)
