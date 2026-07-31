cmake_minimum_required(VERSION 3.24)
project(CellScope VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.4.2)
FetchContent_MakeAvailable(cli11)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.14.1)
FetchContent_GetProperties(spdlog)
if(NOT spdlog_POPULATED)
  cmake_policy(PUSH)
  if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
  endif()
  FetchContent_Populate(spdlog)
  cmake_policy(POP)
endif()
add_library(cellscope_spdlog INTERFACE)
target_include_directories(cellscope_spdlog INTERFACE ${spdlog_SOURCE_DIR}/include)
target_compile_definitions(cellscope_spdlog INTERFACE FMT_CONSTEVAL= FMT_USE_CONSTEVAL=0)

add_library(cellscope_core
  src/core/types.cpp
  src/core/config.cpp
  src/utils/thread_pool.cpp
  src/parser/parser.cpp
  src/analysis/kpi.cpp
  src/analysis/regression.cpp
  src/analysis/analyzer.cpp
  src/timeline/timeline.cpp
  src/report/report.cpp
  src/benchmark/generator.cpp)
target_include_directories(cellscope_core PUBLIC include)
target_link_libraries(cellscope_core PUBLIC cellscope_spdlog)
target_compile_options(cellscope_core PRIVATE
  $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:-Wall -Wextra -Wpedantic>)

add_executable(cellscope src/cli/main.cpp)
target_link_libraries(cellscope PRIVATE cellscope_core CLI11::CLI11 cellscope_spdlog)

option(CELLSCOPE_BUILD_TESTS "Build CellScope tests" ON)
if(CELLSCOPE_BUILD_TESTS)
  enable_testing()
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  add_executable(cellscope_tests
    tests/parser_tests.cpp
    tests/analysis_tests.cpp
    tests/report_tests.cpp)
  target_link_libraries(cellscope_tests PRIVATE cellscope_core GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(cellscope_tests)
endif()

install(TARGETS cellscope RUNTIME DESTINATION bin)
