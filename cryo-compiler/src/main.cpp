#include <iostream>
#include <string>

#include <argparse/argparse.hpp>

int main(int argc, char ** argv) {
    argparse::ArgumentParser parser("cryoc", "0.0.0", argparse::default_arguments::help);
    parser.add_argument("-i", "--input_files")
        .required()
        .help("List of cryo source files to be compiled")
        .nargs(argparse::nargs_pattern::at_least_one);
    parser.add_argument("-o", "--output_file")
        .help("Output file")
        .default_value("out.crye");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    return 0;
}
