#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Token.h"

namespace cryo::parser {

    struct Node {
        virtual ~Node() = default;
    };

    std::string node_to_string(const Node* node);

    struct IdentifierNode : public Node {
        IdentifierNode(Token id)
            : Identifier(std::move(id)) {
            if (id.Type != TokenType::IDENTIFIER) {
                throw std::logic_error("Identifier node cannot be created with non identifier token!");
            }
        }
        ~IdentifierNode() override = default;

        Token Identifier;
    };

    struct NodeBlock : public Node {
        ~NodeBlock() override = default;

        std::vector<std::unique_ptr<Node>> Block;
    };

    struct ScopeNode : public NodeBlock {
        ~ScopeNode() override = default;
    };

    struct VariableNode : public Node {
        ~VariableNode() override = default;

        std::unique_ptr<IdentifierNode> TypeIdentifier;
        std::unique_ptr<IdentifierNode> VariableIdentifier;
    };

    struct VariableDeclarationNode : public VariableNode {
        ~VariableDeclarationNode() override = default;
    };

    struct BinaryOperation : public Node {
        ~BinaryOperation() override = default;

        std::unique_ptr<Node> LeftValue;
        Token Operator;
        std::unique_ptr<Node> RightValue;
    };

    struct UnaryOperation : public Node {
        ~UnaryOperation() override = default;

        Token Operator;
        std::unique_ptr<Node> Value;
    };

    struct AssignmentOperation : public Node {
        ~AssignmentOperation() override = default;

        std::unique_ptr<IdentifierNode> LeftValue;
        Token Operator;
        std::unique_ptr<Node> RightValue;
    };

    struct FunctionDefinitionNode : public Node {
        ~FunctionDefinitionNode() override = default;

        std::unique_ptr<IdentifierNode> Identifier;
        std::vector<std::unique_ptr<VariableDeclarationNode>> Parameters;
        std::unique_ptr<IdentifierNode> ReturnType;

        std::unique_ptr<ScopeNode> Body;
    };

    struct IntegerLiteralNode : public Node {
        IntegerLiteralNode(Token integer)
            : Value(std::move(integer)) {
            if (Value.Type != TokenType::INT) {
                throw std::logic_error("IntegerLiteralNode cannot be created with non int token!");
            }
        }
        ~IntegerLiteralNode() override = default;

        Token Value;
    };

    struct FloatLiteralNode : public Node {
        FloatLiteralNode(Token f)
            : Value(std::move(f)) {
            if (Value.Type != TokenType::FLOAT) {
                throw std::logic_error("FloatLiteralNode cannot be created with non float token!");
            }
        }
        ~FloatLiteralNode() override = default;

        Token Value;
    };

    struct StringLiteralNode : public Node {
        StringLiteralNode(Token string)
            : Value(std::move(string)) {
            if (Value.Type != TokenType::STRING) {
                throw std::runtime_error("StringLiteralNode cannot be created with non string token!");
            }
        }
        ~StringLiteralNode() override = default;

        Token Value;
    };

    struct BoolLiteralNode : public Node {
        BoolLiteralNode(Token b)
            : Value(b.Type == TokenType::TRUE ? true : false), LexerToken(std::move(b)) {
            if (LexerToken.Type != TokenType::TRUE && LexerToken.Type != TokenType::FALSE) {
                throw std::logic_error("BoolLiteralNode cannot be created with non TRUE || FALSE token!");
            }
        }
        ~BoolLiteralNode() override = default;

        bool Value;
        Token LexerToken;
    };

}
