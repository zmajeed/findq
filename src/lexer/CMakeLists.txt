# findq/lexer/CMakeLists.txt

project(findq_lexer)

# tests

add_executable(findq_lexer.gtest findq_lexer.gtest.cpp)

target_compile_definitions(findq_lexer.gtest PRIVATE $<$<PLATFORM_ID:CYGWIN>: GTEST_HAS_PTHREAD=1 _POSIX_C_SOURCE=200809L _GNU_SOURCE>)

target_compile_options(findq_lexer.gtest PRIVATE -Wall -Werror -Wextra -std=c++26 -pthread -fconcepts-diagnostics-depth=10)
target_compile_options(findq_lexer.gtest PRIVATE $<$<CONFIG:debug>: -O0 -ggdb3> $<$<CONFIG:release>: -Og>)

target_link_libraries(findq_lexer.gtest flexbisonlib.findqparser gmock_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(findq_lexer.gtest DISCOVERY_MODE PRE_TEST EXTRA_ARGS --gtest_color=yes)

