cmake_minimum_required(VERSION 3.27)
project(Files_Organizations)

set(CMAKE_CXX_STANDARD 14)

include_directories(back-end/data_structures)
include_directories(back-end/parser)
include_directories(back-end/parserSequential)

add_executable(Files_Organizations
        back-end/data_structures/main.cpp
        back-end/data_structures/sequentialFile.cpp
        back-end/data_structures/sequentialFile.h
        back-end/parser/main.cpp
        back-end/parser/normal.hpp
        back-end/parser/Parser.hpp
        back-end/parser/query.hpp
        back-end/parser/record.hpp
        back-end/parser/record_seq.hpp
        back-end/parser/Scanner.hpp
        back-end/parser/sequential_file.hpp
        back-end/parser/Token.hpp
        back-end/parserSequential/main.cpp
        back-end/parserSequential/Parser.hpp
        back-end/parserSequential/Scanner.hpp
        back-end/parserSequential/Token.hpp
        data_structures/avl/avl.cpp)
