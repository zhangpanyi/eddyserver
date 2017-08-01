# 工程名称
set(CURRENT_PROJECT_NAME eddyserver)

# 添加编译列表
set(CURRENT_PROJECT_SRC_LISTS 
  eddyserver/thread_pool.cpp
  eddyserver/net_message.cpp
  eddyserver/io_service_thread.cpp
  eddyserver/io_service_thread_manager.cpp
  eddyserver/message_filter.cpp
  eddyserver/tcp_client.cpp
  eddyserver/tcp_server.cpp
  eddyserver/tcp_session.cpp
  eddyserver/tcp_session_handler.cpp
  eddyserver/tcp_session_queue.cpp
)

# 包含目录
include_directories(
  ${ASIO_INCLUDE_DIRS}
)

# 链接目录
link_directories(
  ${BINARY_OUTPUT_DIR}
)

# 配置生成库
if (WIN32)
  add_library(${CURRENT_PROJECT_NAME} STATIC
    ${CURRENT_PROJECT_SRC_LISTS}
  )
else()
  add_library(${CURRENT_PROJECT_NAME} SHARED
    ${CURRENT_PROJECT_SRC_LISTS}
  )
endif()

set_target_properties(${CURRENT_PROJECT_NAME}
  PROPERTIES
  OUTPUT_NAME ${EDDYSERVER_LIBRARY}
  ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
)
