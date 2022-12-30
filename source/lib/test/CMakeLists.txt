cmake_minimum_required(VERSION 3.13)

message(STATUS "Fetching googletest")

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/58d77fa8070e8cec2dc1ed015d66b454c8d78850.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  unitTestSuite
  firstDummyTest.cpp
)

target_link_libraries(
    unitTestSuite
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unitTestSuite)
