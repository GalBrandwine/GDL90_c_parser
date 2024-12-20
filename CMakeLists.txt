cmake_minimum_required(VERSION 3.5.0)
project(air_sample_task VERSION 0.1.0 LANGUAGES C)

add_library(${PROJECT_NAME}_lib src/air_sample_task.c
    src/crc16.c)

target_include_directories(${PROJECT_NAME}_lib PUBLIC
    include
    src/)

##################################################################### 
#                                                                   #
#Crate a main program - intrudocing how to use this parser library  #
#                                                                   #
#####################################################################
add_executable(main
main.c)

target_include_directories(main PRIVATE 
include)
target_link_libraries(main ${PROJECT_NAME}_lib)


##################################################################### 
#                                                                   #
#   Create unittests                                                #
#                                                                   #
#####################################################################

IF( DEFINED build_test )

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

include(CTest)
enable_testing()



# Tests 1
add_executable(test_gdl90_parser
    tests/test_gdl90_parser.c
)

target_include_directories(test_gdl90_parser PRIVATE
    include
    src/
    ${unity_SOURCE_DIR}/src
)
target_link_libraries(test_gdl90_parser
    ${PROJECT_NAME}_lib
    ${unity_BINARY_DIR}/libunity.a
)

# Run all tests     - ctest -j14 -C Debug -T test --output-on-failure
# Run specific test - ctest -j14 -C Debug -T test --output-on-failure -R ^test_gdl90_parser
add_test(test_gdl90_parser test_gdl90_parser)

# Tests 2
add_executable(test_gdl90_parser_heartbeat_message
    tests/test_gdl90_parser_heartbeat_message.c
)

target_include_directories(test_gdl90_parser_heartbeat_message PRIVATE
    include
    src/
    ${unity_SOURCE_DIR}/src
)
target_link_libraries(test_gdl90_parser_heartbeat_message
    ${PROJECT_NAME}_lib
    ${unity_BINARY_DIR}/libunity.a
)
add_test(test_gdl90_parser_heartbeat_message test_gdl90_parser_heartbeat_message)


# Tests 3
add_executable(test_gdl90_parser_air_extension_message
    tests/test_gdl90_parser_air_extension_message.c
)

target_include_directories(test_gdl90_parser_air_extension_message PRIVATE
    include
    src/
    ${unity_SOURCE_DIR}/src
)
target_link_libraries(test_gdl90_parser_air_extension_message
    ${PROJECT_NAME}_lib
    ${unity_BINARY_DIR}/libunity.a
)
add_test(test_gdl90_parser_air_extension_message test_gdl90_parser_air_extension_message)
ENDIF( DEFINED build_test )