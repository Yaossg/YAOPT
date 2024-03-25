cmake_minimum_required(VERSION 3.22.1)
project(YAOPT)

set(CMAKE_CXX_STANDARD 20)

add_executable(YAOPT main.cpp util.hpp entity.hpp inst.hpp
        parser.hpp parser.cpp lexer.hpp lexer.cpp token.hpp diagnostics.hpp diagnostics.cpp source.hpp source.cpp)
