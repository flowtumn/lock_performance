set(TEST_PROJECT_NAME ${PROJECT_NAME}-test)

set(SOURCES
  test_main.cpp
)

add_executable(
  ${TEST_PROJECT_NAME}
  $<TARGET_OBJECTS:${PROJECT_NAME}>
  ${SOURCES}
)

include_directories (${INCLUDE_ROOT_DIR})

add_test(test ${TEST_PROJECT_NAME})
