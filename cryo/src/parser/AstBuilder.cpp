#include "cryopch.h"
#include "AstBuilder.h"

#include <complex>
#include <stack>
#include <utility>

#include "ParserError.h"

namespace cryo::parser {

    AstBuilder::AstBuilder(std::filesystem::path file, std::shared_ptr<std::vector<Token>> tokens, std::shared_ptr<std::string> source)
        : m_File(std::move(file)), m_Tokens(std::move(tokens)), m_Source(std::move(source)) {
    }

    std::pair<std::unique_ptr<NodeBlock>, ErrorQueue> AstBuilder::build_tree() {
        auto node_block = std::make_unique<NodeBlock>();
        m_CurrentToken = 0;
        m_ErrorQueue.clean();

        for (const Token* token = &peek(); token->Type != TokenType::END_OF_FILE; token = &peek()) {
            if (token->Type == TokenType::FN) {
                if (auto func = build_function_ast(); func != nullptr) {
                    node_block->Block.emplace_back(std::move(func));
                }
                continue;
            }
            advance();
        }

        return std::make_pair(std::move(node_block), m_ErrorQueue);
    }

    std::unique_ptr<FunctionDefinitionNode> AstBuilder::build_function_ast() {
        auto id = advance(); // The token after 'fn' has to be the function identifier
        if (id.Type == TokenType::END_OF_FILE) {
            push_error(CE_UNEXPECTED_END, "Expected identifier after keyword 'fn', found nothing!");
            return nullptr;
        }
        if (id.Type != TokenType::IDENTIFIER) {
            push_error(CE_INVALID_TOKEN, "Expected identifier after keyword 'fn'!");
            return nullptr;
        }

        auto function_node = std::make_unique<FunctionDefinitionNode>();
        function_node->Identifier = std::make_unique<IdentifierNode>(id);

        if (const auto& open_param = advance(); open_param.Type != TokenType::END_OF_FILE) {
            if (open_param.Type != TokenType::LEFT_PAREN) {
                push_error(CE_UNEXPECTED_END, "Expected Parameters declaration after function identifier, missing '('!");
                return nullptr;
            }

            // TODO: Parameters

            // Close parameters
            if (const auto close_param = advance(); close_param.Type != TokenType::END_OF_FILE) {
                if (close_param.Type != TokenType::RIGHT_PAREN) {
                    push_error(CE_INVALID_TOKEN, "Function parameters where never closed!");
                    return nullptr;
                }
            }
            else {
                push_error(CE_UNEXPECTED_END, "Function parameters where never closed!");
                return nullptr;
            }

        }
        else {
            push_error(CE_UNEXPECTED_END, "Expected Parameters declaration after function identifier, found nothing!");
            return nullptr;
        }

        // TODO: Return types
        if (auto body = build_function_body_ast(function_node->Identifier->Identifier.lexeme); body != nullptr) {
            function_node->Body = std::move(body);
        }
        else {
            return nullptr;
        }

        return function_node;
    }

    std::unique_ptr<ScopeNode> AstBuilder::build_function_body_ast(const std::string& id) {
        if (const auto open_brace = advance(); open_brace.Type != TokenType::END_OF_FILE) {
            if (open_brace.Type != TokenType::LEFT_BRACE) {
                push_error(CE_INVALID_TOKEN, std::format("Function {} body was never opened!", id));
                return nullptr;
            }

            return build_scope_node();

        }
        push_error(CE_UNEXPECTED_END, std::format("Function {} Missing body!", id));
        return nullptr;
    }

    std::unique_ptr<ScopeNode> AstBuilder::build_scope_node()
    {
        auto body = std::make_unique<ScopeNode>();
        std::stack<ScopeNode*> scope_stack;
        scope_stack.push(body.get());

        while (advance().Type != TokenType::END_OF_FILE && !scope_stack.empty()) {
            switch (peek().Type) {
            case TokenType::LEFT_BRACE: {
                auto* scope = dynamic_cast<ScopeNode*>
                    (scope_stack.top()->Block.emplace_back(std::make_unique<ScopeNode>()).get());
                scope_stack.push(scope);
                break;
            }

            case TokenType::RIGHT_BRACE: {
                scope_stack.pop();
                break;
            }

            case TokenType::IDENTIFIER:
            case TokenType::INT:
            case TokenType::FLOAT:
            case TokenType::STRING: {
                if (auto node = build_expression_ast(); node != nullptr) {
                    scope_stack.top()->Block.emplace_back(std::move(node));
                }
                break;
            }

            case TokenType::VAR: {
                if (auto var_decl = build_variable_declaration_ast(); var_decl != nullptr) {
                    scope_stack.top()->Block.emplace_back(std::move(var_decl));
                }
                break;
            }

            case TokenType::IF: {
                if (auto if_state = build_if_statement_node(); if_state != nullptr) {
                    scope_stack.top()->Block.emplace_back(std::move(if_state));
                }
                break;
            }

            case TokenType::WHILE: {
                if (auto while_state = build_while_statement_node(); while_state != nullptr) {
                    scope_stack.top()->Block.emplace_back(std::move(while_state));
                }
                break;
            }

            case TokenType::SEMICOLON: break;

                // Temporary
            case TokenType::PRINT: {
                if (auto print = build_print_ast(); print != nullptr) {
                    scope_stack.top()->Block.emplace_back(std::move(print));
                }
                break;
            }

            default: {
                push_error(CE_INVALID_TOKEN,
                    std::format("Unexpected token of type {}!", TokenType_to_string(peek().Type)));
                return nullptr;
            };
            }
        }
        retreat();

        return body;
    }

    std::unique_ptr<Node> AstBuilder::build_variable_declaration_ast() {
        auto var_decl = std::make_unique<VariableDeclarationNode>();
        const auto var_id = advance();

        if (var_id.Type == TokenType::END_OF_FILE) {
            push_error(CE_UNEXPECTED_END, "Expected variable identifier after keyword 'var', found nothing!");
            return nullptr;
        }
        if (var_id.Type != TokenType::IDENTIFIER) {
            push_error(CE_INVALID_TOKEN, "Expected variable identifier after keyword 'var'!");
            return nullptr;
        }

        if (const auto var_type_operator = advance(); var_type_operator.Type != TokenType::END_OF_FILE) {
            if (var_type_operator.Type != TokenType::COLON) {
                push_error(CE_INVALID_TOKEN, "Expected type operator ':' after variable identifier!");
                return nullptr;
            }
        } else {
            push_error(CE_UNEXPECTED_END, "Expected type operator ':' after variable identifier, found nothing!");
            return nullptr;
        }

        auto& var_type = advance();
        if (var_type.Type == TokenType::END_OF_FILE) {
            push_error(CE_UNEXPECTED_END, "Expected type identifier after type operator ':', found nothing!");
            return nullptr;
        }
        if (var_type.Type != TokenType::IDENTIFIER) {
            push_error(CE_INVALID_TOKEN, "Expected type identifier after type operator ':'!");
            return nullptr;
        }

        var_decl->VariableIdentifier = std::make_unique<IdentifierNode>(var_id);
        var_decl->TypeIdentifier = std::make_unique<IdentifierNode>(var_type);

        switch (auto& semicolon_or_assign = advance(); semicolon_or_assign.Type) {
            case TokenType::SEMICOLON: {
                return var_decl;
            }

            case TokenType::EQUAL: {
                auto block = std::make_unique<NodeBlock>();
                block->Block.emplace_back(std::move(var_decl));

                auto assignment = std::make_unique<AssignmentOperation>();
                assignment->Operator = semicolon_or_assign;
                assignment->LeftValue = std::make_unique<IdentifierNode>(var_id);
                advance();
                assignment->RightValue = build_expression_ast();
                if (assignment->RightValue == nullptr) {
                    return nullptr;
                }

                block->Block.emplace_back(std::move(assignment));

                return block;
            }

            case TokenType::END_OF_FILE: {
                push_error(CE_UNEXPECTED_END, "Expected semicolon or assignment after variable declaration, found end!", &var_id);
                return nullptr;
            }

            default: {
                push_error(CE_INVALID_TOKEN, "Expected semicolon or assignment after variable declaration!");
                return nullptr;
            }
        }
    }

    std::unique_ptr<Node> AstBuilder::build_expression_ast() {
        if (const auto result = get_expression_tokens(); result.has_value()) {
            return build_expression_component_ast(result.value());
        }

        return nullptr;
    }

    std::span<const Token> remove_useless_paren(const std::span<const Token>& expr) {
        std::span<const Token> s = expr;
        while (s.begin()->Type == TokenType::LEFT_PAREN && s[s.size() - 1].Type == TokenType::RIGHT_PAREN) {
            s = std::span(expr.data() + 1, expr.size() - 2);
        }
        return s;
    }

    std::unique_ptr<Node> AstBuilder::build_expression_component_ast(std::span<const Token> tokens) {
        if (tokens.size() == 1) {
            switch (tokens.begin()->Type) {
                case TokenType::IDENTIFIER: {
                    return std::make_unique<IdentifierNode>(*tokens.begin());
                }
                case TokenType::STRING: {
                    return std::make_unique<StringLiteralNode>(*tokens.begin());
                }
                case TokenType::INT: {
                    return std::make_unique<IntegerLiteralNode>(*tokens.begin());
                }
                case TokenType::FLOAT: {
                    return std::make_unique<FloatLiteralNode>(*tokens.begin());
                }
                case TokenType::TRUE:
                case TokenType::FALSE: {
                    return std::make_unique<BoolLiteralNode>(*tokens.begin());
                }

                default:
                    push_error(CE_INVALID_TOKEN, std::format("Expected expression, found {}!", TokenType_to_string(tokens.begin()->Type)));
                    return nullptr;
            }
        }

        uint32_t op;
        if (const auto result = get_least_priority_operator(tokens); result.has_value()) {
            op = result.value();
        } else { // No operators with more than 1 token should mean a function call
            // TODO: function call
            return nullptr;
        }

        switch (const Token& operator_token = tokens[op]; operator_token.Type) {
            // Binary operators
            case TokenType::MINUS: {
                if (op == 0) { // Minus with no left value means: 0 - right_value
                    auto op_node = std::make_unique<BinaryOperation>();
                    op_node->Operator = operator_token;
                    op_node->LeftValue = std::make_unique<FloatLiteralNode>(Token{"0", 0, 0, TokenType::FLOAT});
                    op_node->RightValue = build_expression_component_ast(
                        remove_useless_paren(std::span(tokens.data() + op + 1, tokens.size() - op - 1))
                    );
                    if (op_node->RightValue == nullptr) {
                        return nullptr;
                    }
                    
                    return op_node;
                }
            }
            case TokenType::PLUS:
            case TokenType::SLASH:
            case TokenType::ASTERISK:
            case TokenType::BANG_EQUAL:
            case TokenType::EQUAL_EQUAL:
            case TokenType::GREATER:
            case TokenType::GREATER_EQUAL:
            case TokenType::LESS:
            case TokenType::LESS_EQUAL:
            case TokenType::AND:
            case TokenType::AND_AND:
            case TokenType::OR:
            case TokenType::OR_OR: {
                if (op == 0 || op == tokens.size() - 1) {
                    push_error(CE_EMPTY_EXPRESSION, std::format("Expected expression for operator {}!", TokenType_to_string(tokens[op].Type)));
                    return nullptr;
                }

                auto op_node = std::make_unique<BinaryOperation>();
                op_node->Operator = operator_token;
                op_node->LeftValue = build_expression_component_ast(remove_useless_paren(std::span(tokens.data(), op)));
                op_node->RightValue = 
                    build_expression_component_ast(remove_useless_paren(std::span(tokens.data() + op + 1, tokens.size() - op - 1)));
                if (op_node->LeftValue == nullptr || op_node->RightValue == nullptr) {
                    return nullptr;
                }
                return op_node;
            }

            // Unary operators
            case TokenType::BANG: {
                if (op == tokens.size() - 1) {
                    push_error(CE_EMPTY_EXPRESSION, std::format("Expected expression for operator {}!", TokenType_to_string(tokens[op].Type)),
                        tokens.data() + op);
                    return nullptr;
                }

                auto op_node = std::make_unique<UnaryOperation>();
                op_node->Operator = operator_token;
                op_node->Value = build_expression_component_ast(remove_useless_paren(std::span(tokens.data() + op + 1, tokens.size() - op - 1)));
                return op_node->Value != nullptr ? std::move(op_node) : nullptr;
            }

            case TokenType::EQUAL: {
                auto ass = std::make_unique<AssignmentOperation>();
                ass->Operator = operator_token;
                auto left = remove_useless_paren(std::span(tokens.data(), op));
                if (left.size() != 1 || left[0].Type != TokenType::IDENTIFIER) {
                    // TODO: Error
                    return nullptr;
                }
                ass->LeftValue = std::make_unique<IdentifierNode>(left[0]);
                ass->RightValue = build_expression_component_ast(remove_useless_paren(std::span(tokens.data() + op + 1, tokens.size() - op - 1)));
                return ass;
            }

            default: {
                throw std::logic_error("Expected operator_token to be an operator!");
                return nullptr;
            }
        }
    }

    std::unique_ptr<Node> AstBuilder::build_if_statement_node()
    {
        auto if_then_else = std::make_unique<IfThenElseNode>();

        auto& open_param = advance();
        if (open_param.Type != TokenType::LEFT_PAREN) {
            push_error(CE_UNEXPECTED_TOKEN, "if Statement expects a condition in between parenthesis!");
            return nullptr;
        }

        uint32_t paren_count = 1;
        uint32_t expr_size = 0;
        while (advance().Type != TokenType::END_OF_FILE) {
            if (peek().Type == TokenType::LEFT_PAREN) {
                paren_count++;
            }
            else if (peek().Type == TokenType::RIGHT_PAREN) {
                paren_count--;
            }

            if (paren_count == 0) {
                break;
            }

            expr_size++;
        }
        if (paren_count != 0) {
            push_error(CE_UNEXPECTED_END, "Unclosed condition for if statement!", &open_param);
        }
        advance();

        if_then_else->Condition = build_expression_component_ast(std::span(&open_param + 1, expr_size));
        if (if_then_else->Condition == nullptr) {
            return nullptr;
        }
        
        if (peek().Type != TokenType::LEFT_BRACE) {
            push_error(CE_UNEXPECTED_TOKEN, "if statement missing body!");
            return nullptr;
        }

        if_then_else->IF = build_scope_node();
        if (if_then_else->IF == nullptr) {
            return nullptr;
        }
        advance();

        if (peek().Type != TokenType::ELSE) {
            return if_then_else;
        }

        if (advance().Type == TokenType::IF) {
            if_then_else->ELSE = build_if_statement_node();
        }
        else if (peek().Type == TokenType::LEFT_BRACE) {
            if_then_else->ELSE = build_scope_node();
        }
        else {
            push_error(CE_UNEXPECTED_TOKEN, "else statment missing body!");
            return nullptr;
        }

        if (if_then_else->ELSE == nullptr) {
            return nullptr;
        }

        return if_then_else;
    }

    std::unique_ptr<Node> AstBuilder::build_while_statement_node() {
        auto while_state = std::make_unique<WhileNode>();

        auto& open_param = advance();
        if (open_param.Type != TokenType::LEFT_PAREN) {
            push_error(CE_UNEXPECTED_TOKEN, "while Statement expects a condition in between parenthesis!");
            return nullptr;
        }

        uint32_t paren_count = 1;
        uint32_t expr_size = 0;
        while (advance().Type != TokenType::END_OF_FILE) {
            if (peek().Type == TokenType::LEFT_PAREN) {
                paren_count++;
            }
            else if (peek().Type == TokenType::RIGHT_PAREN) {
                paren_count--;
            }

            if (paren_count == 0) {
                break;
            }

            expr_size++;
        }
        if (paren_count != 0) {
            push_error(CE_UNEXPECTED_END, "Unclosed condition for while statement!", &open_param);
        }
        advance();

        while_state->Condition = build_expression_component_ast(std::span(&open_param + 1, expr_size));
        if (while_state->Condition == nullptr) {
            return nullptr;
        }

        if (peek().Type != TokenType::LEFT_BRACE) {
            push_error(CE_UNEXPECTED_TOKEN, "while statement missing body!");
            return nullptr;
        }

        while_state->Body = build_scope_node();
        if (while_state->Body == nullptr) {
            return nullptr;
        }

        return while_state;
    }

    std::unique_ptr<Node> AstBuilder::build_print_ast()
    {
        auto print_node = std::make_unique<PrintNode>();
        advance();

        auto expr = build_expression_ast();
        if (expr == nullptr) {
            return nullptr;
        }
        print_node->Value = std::move(expr);

        return std::move(print_node);
    }

    std::optional<std::span<Token>> AstBuilder::get_expression_tokens() {
        const uint32_t start = m_CurrentToken;
        uint32_t size = 0;

        uint32_t open_braces = 0;
        bool found_semicolon = false;
        for (auto* token = &peek(); token->Type != TokenType::END_OF_FILE; token = &advance()) {
            if (token->Type == TokenType::SEMICOLON) {
                found_semicolon = true;
                break;
            }
            if (token->Type == TokenType::LEFT_BRACE) {
                open_braces++;
            } else if (token->Type == TokenType::RIGHT_BRACE) {
                if (open_braces == 0) { // Hit the end of something before finding the semicolon, abort expression to avoid cascading errors
                    retreat();
                    break;
                }
                open_braces--;
            }

            size++;
        }

        if (!found_semicolon) {
            push_error(CE_UNEXPECTED_END, "Expression was never closed, did you forget a semicolon?", &m_Tokens->at(start));
            return {};
        }
        if (size == 0) {
            push_error(CE_EMPTY_EXPRESSION, "Expression is empty!", &m_Tokens->at(start - 1));
            return {};
        }

        return std::span<Token>(m_Tokens->data() + start, size);
    }

    std::map<TokenType, uint32_t> s_operator_reverse_order = {
        // Assignment
        { TokenType::EQUAL, 0 },

        // Logical OR
        { TokenType::OR_OR, 1 },

        // Logical AND
        { TokenType::AND_AND, 2 },

        // Bitwise OR
        { TokenType::OR, 3 },

        // Bitwise AND
        { TokenType::AND, 5 },

        // Comparison == and !=
        { TokenType::BANG_EQUAL, 6 },
        { TokenType::EQUAL_EQUAL, 6 },

        // Comparison <, <=, >, >=
        { TokenType::GREATER, 7 },
        { TokenType::GREATER_EQUAL, 7 },
        { TokenType::LESS, 7 },
        { TokenType::LESS_EQUAL, 7 },

        // Addition and subtraction
        { TokenType::PLUS, 8 },
        { TokenType::MINUS, 8 },

        // Multiplication, division and remainder
        { TokenType::ASTERISK, 9 },
        { TokenType::SLASH, 9 },
        { TokenType::REMAINDER, 9 },

        // Logical not
        { TokenType::BANG, 10 },
    };

    std::optional<uint32_t> AstBuilder::get_least_priority_operator(const std::span<const Token> expression) {
        uint32_t current_paren_level = 0;

        bool found_op = false;
        uint32_t current_lowest_op = 0;
        uint32_t current_lowest_op_order = std::numeric_limits<uint32_t>::max();
        uint32_t current_lowest_op_paren_lvl = 0;
        for (uint32_t i = 0; i < expression.size(); i++) {
            const auto& token = expression[i];
            if (token.Type == TokenType::LEFT_PAREN) {
                current_paren_level++;
            } else if (token.Type == TokenType::RIGHT_PAREN) {
                if (current_paren_level == 0) {
                    push_error(CE_NO_PARENTHESIS_TO_CLOSE, "No parenthesis to be closed!", &token);
                    return {};
                }
                current_paren_level--;
            }

            uint32_t op_lvl;
            if (auto ite = s_operator_reverse_order.find(token.Type); ite != s_operator_reverse_order.end()) {
                found_op = true;
                op_lvl = ite->second;
            } else {
                continue;
            }

            if (current_paren_level <= current_lowest_op_paren_lvl && op_lvl <= current_lowest_op_order) {
                current_lowest_op_order = op_lvl;
                current_lowest_op_paren_lvl = current_paren_level;
                current_lowest_op = i;
            }
        }

        if (!found_op) {
            return {};
        }

        return current_lowest_op;
    }

    void AstBuilder::push_error(std::string_view error_code, const std::string &error_message, const Token* token) {
        if (token == nullptr) {
            token = &peek();
        }

        m_ErrorQueue.push_error<ParserError>(error_code, error_message, m_File,
                token->LineNumber, *m_Source, token->IndexFromSource, token->lexeme.size());
    }

    const Token & AstBuilder::retreat() {
        if (m_CurrentToken > 0) {
            m_CurrentToken--;
        }

        return m_Tokens->at(m_CurrentToken);
    }

    const Token& AstBuilder::advance() {
        m_CurrentToken++;
        if (m_CurrentToken >= m_Tokens->size()) {
            m_CurrentToken = m_Tokens->size() - 1;
        }

        return m_Tokens->at(m_CurrentToken);
    }

    const Token * AstBuilder::previous() const {
        return m_CurrentToken > 0 ? &m_Tokens->at(m_CurrentToken - 1) : nullptr;
    }

    const Token& AstBuilder::peek() const {
        return m_Tokens->at(m_CurrentToken);
    }

    const Token & AstBuilder::peek_next() const {
        return m_CurrentToken + 1 < m_Tokens->size() ? m_Tokens->at(m_CurrentToken + 1) : m_Tokens->at(m_CurrentToken);
    }
}
