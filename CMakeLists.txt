# 最低版本
cmake_minimum_required(VERSION 2.8)

# 工程名称
project(eddyserver C CXX)

# 编译类型
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# 编译选项
if (LINUX)
  set(CMAKE_CXX_FLAGS
    -g
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wno-unused-parameter
    -Wold-style-cast
    -Woverloaded-virtual
    -Wpointer-arith
    -Wshadow
    -Wwrite-strings
    -std=c++1y
  )
endif()

# 输出目录
set(BINARY_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BINARY_OUTPUT_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BINARY_OUTPUT_DIR})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BINARY_OUTPUT_DIR})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
add_definitions(-DBINARY_OUTPUT_DIR="${BINARY_OUTPUT_DIR}")

# 开启分组
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# 配置asio
if (WIN32)
  add_definitions(-DASIO_MSVC)
endif()
add_definitions(-DASIO_STANDALONE)
set(ASIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/asio/asio/include)

# 配置eddyserver
set(EDDYSERVER_LIBRARY eddyserver)
set(EDDYSERVER_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/eddyserver)

# 编译eddyserver
add_subdirectory(eddyserver)

# 编译示例代码
add_subdirectory(examples/echo)
