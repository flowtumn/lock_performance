set(SOURCES
  ${PROJECT_SOURCE_DIR}/src/concurrent_queue.cpp
  ${PROJECT_SOURCE_DIR}/src/ILock.cpp
  ${PROJECT_SOURCE_DIR}/src/releaser.cpp
)

add_library(
  ${PROJECT_NAME}
  OBJECT
  ${SOURCES}
)

include_directories (${INCLUDE_ROOT_DIR})