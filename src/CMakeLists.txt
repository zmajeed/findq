# findql/src/CMakeLists.txt

project(findql_src)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(fqparser.no_actions)

enable_testing()
