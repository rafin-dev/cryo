#pragma once

#include "parser/AST.h"
#include "CryoStack.h"

#include <stack>

namespace cryo::runtime {

	class CryoThread {
	public:
		CryoThread(const parser::FunctionDefinitionNode* func);
		~CryoThread();

		void run(const std::vector<ExpressionResult>& param = {});

	private:
		enum RuntimeFlags {
			None = 0,
			Continue,
			Break,
			Return
		};

		const parser::FunctionDefinitionNode* m_Function;
		CryoStack m_Stack;

		RuntimeFlags m_Flag = None;
		ExpressionResult m_ReturnValue;

		void execute_node(const parser::Node* node);

		void execute_scope_node(const parser::ScopeNode* scope);
		void execute_node_block(const parser::NodeBlock* block);
	
		void execute_variable_declaration_node(const parser::VariableDeclarationNode* var);
		void execute_assignment_operation_node(const parser::AssignmentOperation* ass);
		void execute_if_then_else_node(const parser::IfThenElseNode* ite);
		void execute_while_loop(const parser::WhileNode* wl);
		void execute_loop(const parser::LoopNode* ln);
		void execute_return_node(const parser::ReturnStatementNode* return_statement);

		void execute_print_node(const parser::PrintNode* print);

		uint64_t get_loop_count(ExpressionResult result);

		std::optional<bool> evaluate_condition(const parser::Node* node);

		std::optional<ExpressionResult> evaluate_expression(const parser::Node* node);
		std::optional<ExpressionResult> evaluate_literal_node(const parser::LiteralNode* node);
		std::optional<ExpressionResult> evaluate_identifier_node(const parser::IdentifierNode* node);
		std::optional<ExpressionResult> evaluate_binary_operation(const parser::BinaryOperation* op);
		std::optional<ExpressionResult> evaluate_unary_operation(const parser::UnaryOperation* op);
	};

}