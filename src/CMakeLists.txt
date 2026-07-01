# findq/src/CMakeLists.txt

project(findq_src)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(grammar)
add_subdirectory(lexer)
add_subdirectory(parser)

enable_testing()
