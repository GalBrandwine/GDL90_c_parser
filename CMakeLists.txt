cmake_minimum_required(VERSION 3.5.0)
project(air_sample_task VERSION 0.1.0 LANGUAGES C)

include(FetchContent)
message("Fetching content: Unity")
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity # https://github.com/google/googletest.git
    GIT_TAG master # origin/main
)

FetchContent_MakeAvailable(unity)
message("unity: ${unity_SOURCE_DIR}")
message("unity build: ${unity_BINARY_DIR}")

# include(unity)
add_library(${PROJECT_NAME}_lib air_sample_task.c
    src/crc16.c)
target_include_directories(${PROJECT_NAME}_lib PRIVATE
    include
    src/)

include(CTest)
enable_testing()

# Tests 1
add_executable(test_gdl_90_parser
    tests/test_gdl_90_parser.c
)

target_include_directories(test_gdl_90_parser PRIVATE
    include
    ${unity_SOURCE_DIR}/src
)
target_link_libraries(test_gdl_90_parser
    ${PROJECT_NAME}_lib
    ${unity_BINARY_DIR}/libunity.a

    # gtest
    # gtest_main
)

# ctest -j14 -C Debug -T test --output-on-failure -R ^test_gdl_90_parser
add_test(test_gdl_90_parser test_gdl_90_parser)