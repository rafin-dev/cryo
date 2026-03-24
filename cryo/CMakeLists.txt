cmake_minimum_required(VERSION 4.1)

project(cryo)

add_executable(cryo src/main.cpp
        src/cryopch.h
        src/cryopch.cpp

        src/shared/Error.h
        src/shared/Error.cpp

        src/parser/Lexer.cpp
        src/parser/Lexer.h
        src/parser/Token.h
        src/parser/ParserError.cpp
        src/parser/ParserError.h
        src/parser/AST.h
        src/parser/AstBuilder.cpp
        src/parser/AstBuilder.h
        src/parser/AST.cpp
        src/parser/Parser.cpp
        src/parser/Parser.h

        src/runtime/CryoContext.h
        src/runtime/CryoContext.cpp
        src/runtime/CryoThread.h
        src/runtime/CryoThread.cpp 
        src/runtime/CryoStack.h 
        src/runtime/CryoStack.cpp
        src/runtime/TypeID.h
        )

set_property(TARGET cryo PROPERTY CXX_STANDARD 23)

include_directories(src)

target_precompile_headers(cryo
    PUBLIC
        src/cryopch.h
)