# 设置工程名
set(CURRENT_PROJECT_NAME echo)

# 添加编译列表
set(CURRENT_PROJECT_SRC_LISTS 
  main.cpp
)

# 包含目录
include_directories(
  ${ASIO_INCLUDE_DIRS}
  ${EDDYSERVER_INCLUDE_DIRS}
)

# 链接目录
link_directories(
  ${BINARY_OUTPUT_DIR}
)

# 生成可执行文件
file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
source_group("Header Files" FILES ${CURRENT_HEADERS}) 
add_executable(${CURRENT_PROJECT_NAME} ${CURRENT_HEADERS} ${CURRENT_PROJECT_SRC_LISTS})

set_target_properties(${CURRENT_PROJECT_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY
  "${BINARY_OUTPUT_DIR}"
)

# 链接库配置
target_link_libraries(${CURRENT_PROJECT_NAME}
  ${EDDYSERVER_LIBRARY}
)

# 设置分组
SET_PROPERTY(TARGET ${CURRENT_PROJECT_NAME} PROPERTY FOLDER "examples")
