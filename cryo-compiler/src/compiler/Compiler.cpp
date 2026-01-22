#include "cryopch.h"
#include "Compiler.h"

#include "AstBuilder.h"
#include "Lexer.h"

namespace cryo::compiler {

    Compiler::Compiler(std::filesystem::path file, std::filesystem::path output)
        : m_FilePath(std::move(file)), m_OutputFile(std::move(output)) {
        std::fstream file_stream(m_FilePath);
        std::stringstream stream;
        stream << file_stream.rdbuf();
        m_Source = std::make_shared<std::string>(stream.str());
    }

    ErrorQueue Compiler::compile() const {
        ErrorQueue errors;

        auto lexer = Lexer(m_Source, m_FilePath);
        auto [tokens, lexer_errors] = lexer.get_tokens();
        errors.push_ErrorQueue(lexer_errors);

        auto ast_builder = AstBuilder(m_FilePath, tokens, m_Source);
        auto [ast_root, ast_errors] = ast_builder.build_tree();
        errors.push_ErrorQueue(ast_errors);

        std::cout << node_to_string(ast_root.get()) << std::endl;

        return errors;
    }
}
