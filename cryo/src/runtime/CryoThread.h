#pragma once

#include "parser/AST.h"
#include "CryoStack.h"

#include <stack>

namespace cryo::runtime {

	class CryoThread {
	public:
		CryoThread(const parser::FunctionDefinitionNode* func);
		~CryoThread();

		void run();

	private:
		const parser::FunctionDefinitionNode* m_Function;
		CryoStack m_Stack;

		void execute_node(const parser::Node* node);

		void execute_scope_node(const parser::ScopeNode* scope);
		void execute_node_block(const parser::NodeBlock* block);
	
		void execute_variable_declaration_node(const parser::VariableDeclarationNode* var);
	};

}