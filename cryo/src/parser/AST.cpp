#include "cryopch.h"
#include "AST.h"

namespace cryo::parser {

    static std::string nodeBlock_to_string(const NodeBlock* block) {
        std::string str;
        for (const auto& node : block->Block) {
            str += node_to_string(node.get()) + ";\n";
        }
        return str;
    }

    static std::string variableNodeDeclaration_to_string(const VariableDeclarationNode* var) {
        return "var " + var->Identifier.lexeme;
    }

    static std::string functionNode_to_string(const FunctionDefinitionNode* func) {
        std::string str = "fn " + func->Identifier->Identifier.lexeme + "( ";
        for (const auto& arg : func->Parameters) {
            str += arg->Identifier.lexeme + ", ";
        }
        str += " )";

        str += "\n{\n" + nodeBlock_to_string(func->Body.get()) + "\n}";
        return str;
    }

    std::string AssignmentNode_to_string(const AssignmentOperation* op) {
        return "(" + node_to_string(op->LeftValue.get()) + ") " + op->Operator.lexeme + " (" + node_to_string(op->RightValue.get()) + ")";
    }

    std::string binaryOperation_to_string(const BinaryOperation* op) {
        return "(" + node_to_string(op->LeftValue.get()) + ") " + op->Operator.lexeme + " (" + node_to_string(op->RightValue.get()) + ")";
    }

    std::string unaryOperationNode_to_string(const UnaryOperation* op) {
        return op->Operator.lexeme + " (" + node_to_string(op->Value.get()) + ")";
    }

    std::string node_to_string(const Node* node) {
        if (const auto int_lit = dynamic_cast<const IntegerLiteralNode*>(node); int_lit != nullptr) {
            return int_lit->Value.lexeme;
        }
        if (const auto float_lit = dynamic_cast<const FloatLiteralNode*>(node); float_lit != nullptr) {
            return float_lit->Value.lexeme;
        }
        if (const auto bool_lit = dynamic_cast<const BoolLiteralNode*>(node); bool_lit != nullptr) {
            return bool_lit->LexerToken.lexeme;
        }
        if (const auto id = dynamic_cast<const IdentifierNode*>(node); id != nullptr) {
            return id->Identifier.lexeme;
        }
        if (const auto var_decl = dynamic_cast<const VariableDeclarationNode*>(node); var_decl != nullptr) {
            return variableNodeDeclaration_to_string(var_decl);
        }
        if (const auto ass = dynamic_cast<const AssignmentOperation*>(node); ass != nullptr) {
            return AssignmentNode_to_string(ass);
        }
        if (const auto bin = dynamic_cast<const BinaryOperation*>(node); bin != nullptr) {
            return binaryOperation_to_string(bin);
        }
        if (const auto un = dynamic_cast<const UnaryOperation*>(node); un != nullptr) {
            return unaryOperationNode_to_string(un);
        }
        if (const auto scope = dynamic_cast<const ScopeNode*>(node); scope != nullptr) {
            return "{\n" + nodeBlock_to_string(scope) + "\n}";
        }
        if (const auto block = dynamic_cast<const NodeBlock*>(node); block != nullptr) {
            return nodeBlock_to_string(block);
        }
        if (const auto func = dynamic_cast<const FunctionDefinitionNode*>(node); func != nullptr) {
            return functionNode_to_string(func);
        }

        throw std::runtime_error("Unknown AST node type!");
        return "";
    }
}
