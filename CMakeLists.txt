cmake_minimum_required(VERSION 3.23)

project(todo)

add_library(todo-lib STATIC
  src/arg_parser.cpp
)
target_include_directories(todo-lib PUBLIC include)

add_executable(todo src/main.cpp)
target_compile_features(todo PRIVATE cxx_std_17)
target_link_libraries(todo PRIVATE todo-lib)


