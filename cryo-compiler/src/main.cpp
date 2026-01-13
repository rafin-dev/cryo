#include <iostream>
#include <string>
#include <print>

#include <argparse/argparse.hpp>

#include "compiler/Lexer.h"
#include "shared/Arguments.h"

int main(int argc, const char ** argv) {
    const cryo::Arguments args(argc, argv);

    std::fstream file(*args.SourceFiles.begin());
    std::stringstream sstream;
    sstream << file.rdbuf();

    cryo::compiler::Lexer lexer(sstream.str(), *args.SourceFiles.begin());
    auto [fst, snd] = lexer.get_tokens();
    snd.log();
}
