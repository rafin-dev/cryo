#pragma once
#include "shared/Error.h"
#include "AST.h"
#include "Token.h"

#include <span>

namespace cryo::parser {

    class AstBuilder {
    public:
        explicit AstBuilder(std::filesystem::path file, std::shared_ptr<std::vector<Token>> tokens, std::shared_ptr<std::string> source);

        std::pair<std::unique_ptr<NodeBlock>, ErrorQueue> build_tree();

    private:
        std::unique_ptr<FunctionDefinitionNode> build_function_ast();
        std::unique_ptr<ScopeNode> build_function_body_ast(const std::string& id);
        std::unique_ptr<ScopeNode> build_scope_node();
        std::unique_ptr<Node> build_variable_declaration_ast(bool is_param = false);
        std::unique_ptr<Node> build_expression_ast();
        std::unique_ptr<Node> build_expression_component_ast(std::span<const Token> tokens);
        std::unique_ptr<Node> build_function_call_ast(const std::span<const Token> function, const std::span<const Token> parameters);

        std::unique_ptr<Node> build_class_definition_ast();

        std::unique_ptr<Node> build_if_statement_node();
        std::unique_ptr<Node> build_while_statement_node();
        std::unique_ptr<Node> build_loop_statement_node();
        std::unique_ptr<Node> build_return_statement_node();

        std::span<const Token> get_condition();
        std::optional<std::span<Token>> get_expression_tokens();
        std::optional<uint32_t> get_least_priority_operator(std::span<const Token> expression);

        void push_error(std::string_view error_code, const std::string& error_message, const Token* token = nullptr);

        const Token& retreat();
        const Token& advance();
        [[nodiscard]] const Token* previous() const;
        [[nodiscard]] const Token& peek() const;
        [[nodiscard]] const Token& peek_next() const;

        std::filesystem::path m_File;

        uint32_t m_CurrentToken = 0;
        std::shared_ptr<std::vector<Token>> m_Tokens;
        std::shared_ptr<std::string> m_Source;
        ErrorQueue m_ErrorQueue;

        static std::map<TokenType, uint32_t> s_operator_reverse_order;
    };

} // cryo
