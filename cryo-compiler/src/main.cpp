#include <iostream>
#include <string>
#include <print>

#include <argparse/argparse.hpp>

#include "compiler/AstBuilder.h"
#include "compiler/Compiler.h"
#include "compiler/Lexer.h"
#include "shared/Arguments.h"

int main(const int argc, const char ** argv) {
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

        std::cout << "[Compiling source file: '" << file << "']" << std::endl;
        auto compiler = cryo::compiler::Compiler(file, std::filesystem::path());
        compiler.compile().log();
    }
}
