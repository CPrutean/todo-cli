cmake_minimum_required(VERSION 3.23)

project(todo)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(todo-lib STATIC
  src/arg_parser.cpp
  src/types.cpp
  src/query_client.cpp
)
target_compile_features(todo-lib PUBLIC cxx_std_17)
target_include_directories(todo-lib PUBLIC include)

add_executable(todo src/main.cpp)
target_compile_features(todo PRIVATE cxx_std_17)
target_link_libraries(todo PRIVATE todo-lib)
