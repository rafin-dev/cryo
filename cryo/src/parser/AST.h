#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Token.h"
#include "runtime/TypeID.h"

namespace cryo::parser {

    struct Node {
        virtual ~Node() = default;
    };

    struct IdentifierNode : public Node {
        IdentifierNode(std::string id)
            : Identifier(std::move(id)) {
        }
        ~IdentifierNode() override = default;

        std::string Identifier;
    };

    struct NodeBlock : public Node {
        ~NodeBlock() override = default;

        std::vector<std::unique_ptr<Node>> Block;
    };

    struct ScopeNode : public NodeBlock {
        ~ScopeNode() override = default;
    };

    struct VariableDeclarationNode : public IdentifierNode {
        VariableDeclarationNode(std::string id)
            : IdentifierNode(std::move(id)) 
        { }
        ~VariableDeclarationNode() override = default;
    };

    struct BinaryOperation : public Node {
        ~BinaryOperation() override = default;

        std::unique_ptr<Node> LeftValue;
        TokenType Operator;
        std::unique_ptr<Node> RightValue;
    };

    struct UnaryOperation : public Node {
        ~UnaryOperation() override = default;

        TokenType Operator;
        std::unique_ptr<Node> Value;
    };

    struct AssignmentOperation : public Node {
        ~AssignmentOperation() override = default;

        std::unique_ptr<Node> LeftValue;
        TokenType Operator;
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
        IntegerLiteralNode(std::string integer)
            : Value(std::move(integer)) {
        }
        ~IntegerLiteralNode() override = default;

        std::string Value;
    };

    struct FloatLiteralNode : public LiteralNode {
        FloatLiteralNode(std::string f)
            : Value(std::move(f)) {
        }
        ~FloatLiteralNode() override = default;

        std::string Value;
    };

    struct CharLiteralNode : public LiteralNode {
        CharLiteralNode(char character) {
            Character = character;
        }
        ~CharLiteralNode() override = default;

        char Character;
    };

    struct StringLiteralNode : public LiteralNode {
        StringLiteralNode(std::string string) 
            : Value(string.data() + 1, string.size() - 2) {
        }
        ~StringLiteralNode() override = default;

        std::string Value;
    };

    struct BoolLiteralNode : public LiteralNode {
        BoolLiteralNode(bool b) {
            Value = b;
        }
        ~BoolLiteralNode() override = default;

        bool Value;
    };

    struct ClassDefinitionNode : public Node {
        ~ClassDefinitionNode() override = default;

        enum Visibility {
            Private = 0,
            Protected,
            Public
        };

        std::unique_ptr<IdentifierNode> ClassIdentifier;
        std::unique_ptr<ScopeNode> Destructor;

        std::vector<std::pair<std::unique_ptr<FunctionDefinitionNode>, Visibility>> Constructors;
        
        std::vector<std::pair<std::unique_ptr<FunctionDefinitionNode>, Visibility>> Methods;
        std::vector<std::pair<std::unique_ptr<FunctionDefinitionNode>, Visibility>> StaticMethods;
        
        std::unordered_map<std::string, std::pair<uint32_t, Visibility>> MembersOffset;
        std::unordered_map<std::string, std::pair<runtime::CryoValue, Visibility>> StaticMembers;
    };

}
