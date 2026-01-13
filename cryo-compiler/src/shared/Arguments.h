#pragma once

namespace cryo {

    struct Arguments {
    public:
        Arguments(int argc, const char** argv);

        std::vector<std::string> SourceFiles;
        std::string OutputPath;
    };

}