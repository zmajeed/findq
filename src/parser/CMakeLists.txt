# findq/parser/CMakeLists.txt

project(findq_parser_bison)

# standalone parser executable with -DBUILD_BISON_MAIN
add_executable(findqparse ${FINDQ_PARSER_BISON_CPP_FILE})

target_compile_definitions(findqparse PRIVATE BUILD_BISON_MAIN)
target_compile_definitions(findqparse PRIVATE $<$<PLATFORM_ID:CYGWIN>: GTEST_HAS_PTHREAD=1 _POSIX_C_SOURCE=200809L _GNU_SOURCE>)

target_compile_options(findqparse PRIVATE -Wall -Werror -Wextra -std=c++26 -pthread -fconcepts-diagnostics-depth=10)
target_compile_options(findqparse PRIVATE $<$<CONFIG:debug>: -O0 -ggdb3> $<$<CONFIG:release>: -Og>)

target_link_libraries(findqparse flexbisonlib.findqparser)

# tests

add_executable(findq_parser.gtest findq_parser.gtest.cpp)

target_compile_definitions(findq_parser.gtest PRIVATE $<$<PLATFORM_ID:CYGWIN>: GTEST_HAS_PTHREAD=1 _POSIX_C_SOURCE=200809L _GNU_SOURCE>)

target_compile_options(findq_parser.gtest PRIVATE -Wall -Werror -Wextra -std=c++26 -pthread -fconcepts-diagnostics-depth=10)
target_compile_options(findq_parser.gtest PRIVATE $<$<CONFIG:debug>: -O0 -ggdb3> $<$<CONFIG:release>: -Og>)

target_link_libraries(findq_parser.gtest flexbisonlib.findqparser gmock_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(findq_parser.gtest DISCOVERY_MODE PRE_TEST EXTRA_ARGS --gtest_color=yes)

