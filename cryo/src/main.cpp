#include "cryopch.h"

#include "parser/AstBuilder.h"
#include "parser/parser.h"
#include "parser/Lexer.h"
#include "runtime/CryoContext.h"

#include <iostream>
#include <string>
#include <print>

int main(const int argc, const char ** argv) {
    if (argc == 1) {
        std::cerr << "Missing source file" << std::endl;
        return -1;
    }

    std::filesystem::path file(argv[1]);
    if (!std::filesystem::exists(file) || !std::filesystem::is_regular_file(file)) {
        std::cerr << "File at [" << file.string() << "] either doesn`t exist or is not a file!" << std::endl;
        return -1;
    }

    cryo::runtime::CryoContext(argv[1]).run("main");
}
