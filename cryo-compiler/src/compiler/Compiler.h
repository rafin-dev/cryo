#pragma once

#include "shared/Error.h"

#include <string>
#include <memory>

namespace cryo::compiler {

    class Compiler {
    public:
        Compiler(std::filesystem::path file, std::filesystem::path output);
        ~Compiler() = default;

        [[nodiscard]] ErrorQueue compile() const;

    private:
        std::filesystem::path m_FilePath;
        std::shared_ptr<std::string> m_Source;
        std::filesystem::path m_OutputFile;
    };

}
