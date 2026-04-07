#pragma once

#include "shared/Error.h"
#include "AST.h"

#include <string>
#include <memory>

namespace cryo::parser {

    class Parser {
    public:
        Parser(std::filesystem::path file);
        ~Parser() = default;

        std::optional<std::unique_ptr<NodeBlock>> parse() const;

    private:
        std::filesystem::path m_FilePath;
        std::shared_ptr<std::string> m_Source;
        std::filesystem::path m_OutputFile;
    };

}
