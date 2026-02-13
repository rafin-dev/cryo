#include "cryopch.h"
#include <iostream>
#include <string>
#include <print>

#include <argparse/argparse.hpp>

#include "parser/AstBuilder.h"
#include "parser/parser.h"
#include "parser/Lexer.h"
#include "shared/Arguments.h"
#include "runtime/CryoContext.h"

int main(const int argc, const char ** argv) {
    cryo::runtime::CryoContext("../../../../cryo-compiler/examples/hello_world.cryo").run("main");
    return 0;

    const cryo::Arguments args(argc, argv);

    for (auto& file : args.SourceFiles) {
        if (!std::filesystem::exists(file)) {
            std::cerr << "File " << file << " does not exist!" << std::endl;
            return -1;
        }
        if (!std::filesystem::is_regular_file(file)) {
            std::cerr << "Path " << file << " does not lead to a regular file!" << std::endl;
            return -1;
        }

        auto result = cryo::parser::Parser(file, std::filesystem::path()).parse();
        if (!result.has_value()) {
            std::cerr << "Failed to parse!" << std::endl;
            return -1;
        }

        // return cryo::runtime::Runtime(std::move(result)).run();
    }
}
