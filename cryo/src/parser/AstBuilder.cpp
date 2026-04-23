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
        m_Tokens;

        for (const Token* token = &peek(); token->Type != TokenType::END_OF_FILE; token = &peek()) {
            if (token->Type == TokenType::FN) {
                if (auto func = build_function_ast(); func != nullptr) {
                    node_block->Block.emplace_back(std::move(func));
                }
                continue;
            }
            else if (token->Type == TokenType::CLASS) {
                if (auto class_def = build_class_definition_ast(); class_def != nullptr) {
                    node_block->Block.emplace_back(std::move(class_def));
                }
            }
            advance();
        }

        return std::make_pair(std::move(node_block), m_ErrorQueue);
    }

    std::unique_ptr<FunctionDefinitionNode> AstBuilder::build_function_ast() {
        auto id = advance(); // The token after a function defining keyword has to be the function identifier
        if (id.Type == TokenType::END_OF_FILE) {
            push_error(CE_UNEXPECTED_END, "Expected identifier after keyword 'fn', found nothing!");
            return nullptr;
        }
        if (id.Type != TokenType::IDENTIFIER) {
            push_error(CE_INVALID_TOKEN, "Expected identifier after keyword 'fn'!");
            return nullptr;
        }

        auto function_node = std::make_unique<FunctionDefinitionNode>();
        function_node->Identifier = std::make_unique<IdentifierNode>(id.lexeme);

        if (const auto& open_param = advance(); open_param.Type != TokenType::END_OF_FILE) {
            if (open_param.Type != TokenType::LEFT_PAREN) {
                push_error(CE_UNEXPECTED_END, "Expected Parameters declaration after function identifier, missing '('!");
                return nullptr;
            }

            // Parameters
            advance();
            while (peek().Type == TokenType::VAR) {
                auto param = build_variable_declaration_ast(true);
                if (!param) {
                    return nullptr;
                }

                // TODO: support for default parameter value
                auto* param_ptr = dynamic_cast<VariableDeclarationNode*>(param.release());
                if (!param_ptr) {
                    return nullptr;
                }
                function_node->Parameters.emplace_back(param_ptr);

                if (peek().Type == TokenType::RIGHT_PAREN) {
                    break;
                }
                else if (peek().Type == TokenType::COMMA) {
                    advance();
                }
            }

            // Close parameters
            if (const auto close_param = peek(); close_param.Type != TokenType::END_OF_FILE) {
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

        if (auto body = build_function_body_ast(function_node->Identifier->Identifier); body != nullptr) {
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

    std::unique_ptr<ScopeNode> AstBuilder::build_scope_node() {
        auto body = std::make_unique<ScopeNode>();
        std::stack<ScopeNode*> scope_stack;
        scope_stack.push(body.get());

        while (!scope_stack.empty() && advance().Type != TokenType::END_OF_FILE) {
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

            case TokenType::LOOP: {
                if (auto loop = build_loop_statement_node(); loop != nullptr) {
                    scope_stack.top()->Block.emplace_back(std::move(loop));
                }
                break;
            }

            case TokenType::RETURN: {
                if (auto rt_st = build_return_statement_node(); rt_st != nullptr) {
                    scope_stack.top()->Block.emplace_back(std::move(rt_st));
                }
                break;
            }

            case TokenType::CONTINUE: {
                if (advance().Type != TokenType::SEMICOLON) {
                    push_error(CE_UNEXPECTED_TOKEN, "continue statement does not take parameters!");
                    break;
                }
                scope_stack.top()->Block.emplace_back(std::make_unique<ContinueStatementNode>());
                break;
            }

            case TokenType::BREAK: {
                if (advance().Type != TokenType::SEMICOLON) {
                    push_error(CE_UNEXPECTED_TOKEN, "break statement does not take parameters!");
                    break;
                }
                scope_stack.top()->Block.emplace_back(std::make_unique<BreakStatementNode>());
                break;
            }

            case TokenType::SEMICOLON: break;

            default: {
                push_error(CE_INVALID_TOKEN,
                    std::format("Unexpected token of type {}!", TokenType_to_string(peek().Type)));
                return nullptr;
            };
            }
        }

        return body;
    }

    std::unique_ptr<Node> AstBuilder::build_variable_declaration_ast(bool is_param) {
        const auto var_id = advance();

        if (var_id.Type == TokenType::END_OF_FILE) {
            push_error(CE_UNEXPECTED_END, "Expected variable identifier after keyword 'var', found nothing!");
            return nullptr;
        }
        if (var_id.Type != TokenType::IDENTIFIER) {
            push_error(CE_INVALID_TOKEN, "Expected variable identifier after keyword 'var'!");
            return nullptr;
        }

        auto var_decl = std::make_unique<VariableDeclarationNode>(var_id.lexeme);

        switch (auto& semicolon_or_assign = advance(); semicolon_or_assign.Type) {
            case TokenType::COMMA:
            case TokenType::RIGHT_PAREN: {
                if (is_param) {
                    return var_decl;
                }
                push_error(CE_INVALID_TOKEN, "Expected semicolon or assignment after variable declaration!");
                return nullptr;
            }

            case TokenType::SEMICOLON: {
                return var_decl;
            }

            case TokenType::EQUAL: {
                auto block = std::make_unique<NodeBlock>();
                block->Block.emplace_back(std::move(var_decl));

                auto assignment = std::make_unique<AssignmentOperation>();
                assignment->Operator = semicolon_or_assign.Type;
                assignment->LeftValue = std::make_unique<IdentifierNode>(var_id.lexeme);
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
                    return std::make_unique<IdentifierNode>(tokens.begin()->lexeme);
                }
                case TokenType::STRING: {
                    return std::make_unique<StringLiteralNode>(tokens.begin()->lexeme);
                }
                case TokenType::INT: {
                    return std::make_unique<IntegerLiteralNode>(tokens.begin()->lexeme);
                }
                case TokenType::FLOAT: {
                    return std::make_unique<FloatLiteralNode>(tokens.begin()->lexeme);
                }
                case TokenType::TRUE: {
                    return std::make_unique<BoolLiteralNode>(true);
                }
                case TokenType::FALSE: {
                    return std::make_unique<BoolLiteralNode>(false);
                }

                default:
                    push_error(CE_INVALID_TOKEN, std::format("Expected expression, found {}!", TokenType_to_string(tokens.begin()->Type)));
                    return nullptr;
            }
        }

        // To be a function call it needs to be at least 3 tokens -> id(...)
        if (tokens.size() > 2 && 
            tokens[0].Type == TokenType::IDENTIFIER && 
            tokens[1].Type == TokenType::LEFT_PAREN &&
            tokens[tokens.size() - 1].Type == TokenType::RIGHT_PAREN) {
            return build_function_call_ast(tokens);
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
                    op_node->Operator = operator_token.Type;
                    op_node->LeftValue = std::make_unique<FloatLiteralNode>("0");
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
                op_node->Operator = operator_token.Type;
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
                op_node->Operator = operator_token.Type;
                op_node->Value = build_expression_component_ast(remove_useless_paren(std::span(tokens.data() + op + 1, tokens.size() - op - 1)));
                return op_node->Value != nullptr ? std::move(op_node) : nullptr;
            }

            case TokenType::PLUS_EQUAL:
            case TokenType::MINUS_EQUAL:
            case TokenType::ASTERISK_EQUAL:
            case TokenType::SLASH_EQUAL:
            case TokenType::EQUAL: {
                auto ass = std::make_unique<AssignmentOperation>();
                ass->Operator = operator_token.Type;

                ass->LeftValue = 
                    build_expression_component_ast(remove_useless_paren(std::span(tokens.data(), op)));

                ass->RightValue = 
                    build_expression_component_ast(remove_useless_paren(std::span(tokens.data() + op + 1, tokens.size() - op - 1)));
                
                return ass;
            }

            case TokenType::COMMA: {
                auto expr_list = std::make_unique<NodeBlock>();

                auto left = 
                    build_expression_component_ast(remove_useless_paren(std::span(tokens.data(), op)));
                if (auto* left_list = dynamic_cast<NodeBlock*>(left.get())) {
                    expr_list->Block = std::move(left_list->Block);
                }
                else {
                    expr_list->Block.emplace_back(std::move(left));
                }

                auto right = 
                    build_expression_component_ast(
                        remove_useless_paren(std::span(tokens.data() + op + 1, tokens.size() - op - 1))
                    );
                if (auto* right_list = dynamic_cast<NodeBlock*>(right.get())) {
                    for (auto& expr : right_list->Block) {
                        expr_list->Block.emplace_back(std::move(expr));
                    }
                    right_list->Block.clear();
                }
                else {
                    expr_list->Block.emplace_back(std::move(right));
                }
                
                return expr_list;
            }

            default: {
                throw std::logic_error("Expected operator_token to be an operator!");
                return nullptr;
            }
        }
    }

    std::unique_ptr<Node> AstBuilder::build_function_call_ast(const std::span<const Token> tokens) {
        auto func_call = std::make_unique<FunctionCallNode>();
        func_call->FuncID = std::make_unique<IdentifierNode>(tokens[0].lexeme);

        auto result = build_expression_component_ast(std::span(tokens.data() + 2, tokens.size() - 3));
        if (!result) {
            return std::move(func_call);
        }

        if (auto* block = dynamic_cast<NodeBlock*>(result.get())) {
            result.release();
            func_call->Arguments.reset(block);
        }
        else {
            func_call->Arguments = std::make_unique<NodeBlock>();
            func_call->Arguments->Block.emplace_back(std::move(result));
        }

        return std::move(func_call);
    }

    std::unique_ptr<Node> AstBuilder::build_class_definition_ast()
    {
        if (advance().Type != TokenType::IDENTIFIER) {
            push_error(CE_UNEXPECTED_TOKEN, "Class definition missing Identifier");
            return nullptr;
        }

        const std::string& id = peek().lexeme;

        if (advance().Type != TokenType::LEFT_BRACE) {
            push_error(CE_UNEXPECTED_TOKEN, "Class definition missing opening bracket!");
            return nullptr;
        }

        auto class_def = std::make_unique<ClassDefinitionNode>();
        class_def->ClassIdentifier = std::make_unique<IdentifierNode>(id);

        auto current_visibility = ClassDefinitionNode::Private;
        while (advance().Type != TokenType::RIGHT_BRACE) {
            switch (peek().Type) {

            case TokenType::PRIVATE: {
                if (advance().Type != TokenType::COLON) {
                    push_error(CE_UNEXPECTED_TOKEN, "Expected ':' after visibility declaration!");
                    return nullptr;
                }
                current_visibility = ClassDefinitionNode::Private;
                break;
            }
            case TokenType::PROTECTED: {
                if (advance().Type != TokenType::COLON) {
                    push_error(CE_UNEXPECTED_TOKEN, "Expected ':' after visibility declaration!");
                    return nullptr;
                }
                current_visibility = ClassDefinitionNode::Protected;
                break;
            }
            case TokenType::PUBLIC: {
                if (advance().Type != TokenType::COLON) {
                    push_error(CE_UNEXPECTED_TOKEN, "Expected ':' after visibility declaration!");
                    return nullptr;
                }
                current_visibility = ClassDefinitionNode::Public;
                break;
            }

            case TokenType::FN: {
                if (auto method = build_function_ast(); method != nullptr) {
                    class_def->Methods.emplace_back(std::pair(std::move(method), current_visibility));
                }
                break;
            }

            case TokenType::END_OF_FILE: {
                push_error(CE_UNEXPECTED_END, "Class definition was never closed");
                return nullptr;
                break;
            }

            default: {
                push_error(CE_UNEXPECTED_TOKEN,
                    std::format("Unexpected token '{}', in class definition '{}'", peek().lexeme, id)
                );
                return nullptr;
            }
            }
        }
        return class_def;
    }

    std::unique_ptr<Node> AstBuilder::build_if_statement_node()
    {
        auto if_then_else = std::make_unique<IfThenElseNode>();

        auto& open_param = advance();
        if (open_param.Type != TokenType::LEFT_PAREN) {
            push_error(CE_UNEXPECTED_TOKEN, "if Statement expects a condition in between parenthesis!");
            return nullptr;
        }

        auto condition = get_condition();

        if_then_else->Condition = build_expression_component_ast(condition);
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

        if (advance().Type != TokenType::ELSE) {
            retreat();
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

        auto condition = get_condition();

        while_state->Condition = build_expression_component_ast(condition);
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

    std::unique_ptr<Node> AstBuilder::build_loop_statement_node() {
        auto loop = std::make_unique<LoopNode>();

        auto& paren_or_brace = advance();

        if (paren_or_brace.Type == TokenType::LEFT_PAREN) { // Loop has count
            auto condition = get_condition();
            loop->Count = build_expression_component_ast(condition);
            if (loop->Count == nullptr) {
                return nullptr;
            }

            auto& brace = peek();
            if (brace.Type != TokenType::LEFT_BRACE) {
                push_error(CE_UNEXPECTED_TOKEN, "loop statement missing body!");
                return nullptr;
            }

            loop->Body = build_scope_node();
            return loop->Body == nullptr ? nullptr : std::move(loop);
        }

        if (paren_or_brace.Type == TokenType::LEFT_BRACE) { // Infinte loop
            loop->Body = build_scope_node();
            return loop->Body == nullptr ? nullptr : std::move(loop);
        }

        push_error(CE_UNEXPECTED_TOKEN, "loop statement missing count and body!");
        return nullptr;
    }

    std::unique_ptr<Node> AstBuilder::build_return_statement_node() {
        auto return_node = std::make_unique<ReturnStatementNode>();
        if (advance().Type == TokenType::SEMICOLON) {
            return return_node;
        }

        return_node->ReturnValue = build_expression_ast();
        if (return_node->ReturnValue == nullptr) {
            return nullptr;
        }

        return return_node;
    }

    std::span<const Token> AstBuilder::get_condition() {
        auto& start = peek();
        
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
            push_error(CE_UNEXPECTED_END, "Unclosed condition!", &start);
        }
        advance();

        return std::span(&start + 1, expr_size);
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
        // Comma
        { TokenType::COMMA, 1 },

        // Assignment
        { TokenType::PLUS_EQUAL, 1 },
        { TokenType::MINUS_EQUAL, 1 },
        { TokenType::SLASH_EQUAL, 1 },
        { TokenType::ASTERISK_EQUAL, 1 },
        { TokenType::EQUAL, 1 },

        // Logical OR
        { TokenType::OR_OR, 2 },

        // Logical AND
        { TokenType::AND_AND, 3 },

        // Bitwise OR
        { TokenType::OR, 4 },

        // Bitwise AND
        { TokenType::AND, 6 },

        // Comparison == and !=
        { TokenType::BANG_EQUAL, 7 },
        { TokenType::EQUAL_EQUAL, 7 },

        // Comparison <, <=, >, >=
        { TokenType::GREATER, 8 },
        { TokenType::GREATER_EQUAL, 8 },
        { TokenType::LESS, 8 },
        { TokenType::LESS_EQUAL, 8 },

        // Addition and subtraction
        { TokenType::PLUS, 9 },
        { TokenType::MINUS, 9 },

        // Multiplication, division and remainder
        { TokenType::ASTERISK, 10 },
        { TokenType::SLASH, 10 },
        { TokenType::REMAINDER, 10 },

        // Logical not
        { TokenType::BANG, 11 },
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
