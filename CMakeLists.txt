cmake_minimum_required(VERSION 2.8.12)

set(PROJECT_NAME lock_performance)

project(${PROJECT_NAME})

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	add_definitions("/Zi /W4")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wall -Wextra")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 -O3 -pthread -Wall -Wextra")
endif ()

enable_testing()

set(INCLUDE_ROOT_DIR "${PROJECT_SOURCE_DIR}/include")

add_subdirectory(src)
add_subdirectory(test)
