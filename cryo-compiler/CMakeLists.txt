cmake_minimum_required(VERSION 4.1)

project(cryoc)

add_executable(cryoc src/main.cpp
        src/cryopch.h
        src/cryopch.cpp
        src/shared/Error.h
        src/shared/Error.cpp
        src/compiler/Lexer.cpp
        src/compiler/Lexer.h
        src/compiler/Token.h
        src/shared/Arguments.cpp
        src/shared/Arguments.h
        src/compiler/CompilerError.cpp
        src/compiler/CompilerError.h
        src/compiler/AST.h
        src/compiler/AstBuilder.cpp
        src/compiler/AstBuilder.h
)

set_property(TARGET cryoc PROPERTY CXX_STANDARD 23)

include_directories(src
        ../vendor/argparse/include)

target_precompile_headers(cryoc
    PUBLIC
        src/cryopch.h
)
