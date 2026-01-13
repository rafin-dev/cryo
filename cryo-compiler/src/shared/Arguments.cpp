#include "cryopch.h"
#include "Arguments.h"

#include <argparse/argparse.hpp>

namespace cryo {

    Arguments::Arguments(int argc, const char **argv) {
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
            std::exit(1);
        }

        SourceFiles = parser.get<std::vector<std::string>>("-i");
        OutputPath = parser.get<std::string>("-o");
    }

} // cryo