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

    struct VariableDeclarationNode : public IdentifierNode {
        VariableDeclarationNode(Token id)
            : IdentifierNode(std::move(id)) 
        { }
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

        std::unique_ptr<Node> LeftValue;
        Token Operator;
        std::unique_ptr<Node> RightValue;
    };

    struct FunctionDefinitionNode : public Node {
        ~FunctionDefinitionNode() override = default;

        std::unique_ptr<IdentifierNode> Identifier;
        std::vector<std::unique_ptr<VariableDeclarationNode>> Parameters;

        std::unique_ptr<ScopeNode> Body;
    };

    struct FunctionCallNode : public Node {
        ~FunctionCallNode() override = default;

        std::unique_ptr<IdentifierNode> FuncID;
        std::unique_ptr<NodeBlock> Arguments = std::make_unique<NodeBlock>();
    };

    struct ReturnStatementNode : public Node {
        ~ReturnStatementNode() override = default;

        std::unique_ptr<Node> ReturnValue;
    };

    struct BreakStatementNode : public Node {
        ~BreakStatementNode() override = default;
    };

    struct ContinueStatementNode : public Node {
        ~ContinueStatementNode() override = default;
    };

    struct IfThenElseNode : public Node {
        ~IfThenElseNode() override = default;

        std::unique_ptr<Node> Condition;
        std::unique_ptr<Node> IF;
        std::unique_ptr<Node> ELSE;
    };

    struct WhileNode : public Node {
        ~WhileNode() override = default;

        std::unique_ptr<Node> Condition;
        std::unique_ptr<Node> Body;
    };

    struct LoopNode : public Node {
    public:
        ~LoopNode() override = default;

        std::unique_ptr<Node> Count;
        std::unique_ptr<Node> Body;
    };

    struct LiteralNode : public Node {
        ~LiteralNode() override = default;
    };

    struct IntegerLiteralNode : public LiteralNode {
        IntegerLiteralNode(Token integer)
            : Value(std::move(integer)) {
            if (Value.Type != TokenType::INT) {
                throw std::logic_error("IntegerLiteralNode cannot be created with non int token!");
            }
        }
        ~IntegerLiteralNode() override = default;

        Token Value;
    };

    struct FloatLiteralNode : public LiteralNode {
        FloatLiteralNode(Token f)
            : Value(std::move(f)) {
            if (Value.Type != TokenType::FLOAT) {
                throw std::logic_error("FloatLiteralNode cannot be created with non float token!");
            }
        }
        ~FloatLiteralNode() override = default;

        Token Value;
    };

    struct CharLiteralNode : public LiteralNode {
        CharLiteralNode(Token character)
            : LexerToken(std::move(character)) {
            Character = LexerToken.lexeme[0];
        }
        ~CharLiteralNode() override = default;

        Token LexerToken;
        char Character;
    };

    struct StringLiteralNode : public LiteralNode {
        StringLiteralNode(Token string)
            : LexerToken(std::move(string)) {
            if (LexerToken.Type != TokenType::STRING) {
                throw std::runtime_error("StringLiteralNode cannot be created with non string token!");
            }
            Value = std::string(LexerToken.lexeme.data() + 1, LexerToken.lexeme.size() - 2);
        }
        ~StringLiteralNode() override = default;

        Token LexerToken;
        std::string Value;
    };

    struct BoolLiteralNode : public LiteralNode {
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
