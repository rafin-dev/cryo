#include <iostream>
#include <string>
#include <print>

#include <argparse/argparse.hpp>

#include "compiler/AstBuilder.h"
#include "compiler/Lexer.h"
#include "shared/Arguments.h"

int main(int argc, const char ** argv) {
    const cryo::Arguments args(argc, argv);

    std::fstream file(*args.SourceFiles.begin());
    std::stringstream stream;
    stream << file.rdbuf();

    auto src = std::make_shared<std::string>(stream.str());
    cryo::compiler::Lexer lexer(src, *args.SourceFiles.begin());
    auto [fst, snd] = lexer.get_tokens();
    snd.log();

    cryo::compiler::AstBuilder builder(*args.SourceFiles.begin(), fst, src);
    auto [tree, errors] = builder.build_tree();
    errors.log();
}
