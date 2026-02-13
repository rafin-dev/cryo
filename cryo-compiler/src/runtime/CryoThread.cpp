#include "cryopch.h"
#include "CryoThread.h"

#include "TypeID.h"

namespace cryo::runtime {

	CryoThread::CryoThread(const parser::FunctionDefinitionNode* func) 
		: m_Function(func) {
	}

	CryoThread::~CryoThread() {
	}

	void CryoThread::run() {
		execute_node_block(m_Function->Body.get());
	}

#define CHECK_NODE_TYPE(var, Type, node) const auto* var = dynamic_cast<const Type*>(node); var != nullptr

	void CryoThread::execute_node(const parser::Node* node) {
		if (CHECK_NODE_TYPE(var, parser::VariableDeclarationNode, node)) {
			execute_variable_declaration_node(var);
		}
		else if (CHECK_NODE_TYPE(scope, parser::ScopeNode, node)) {
			execute_scope_node(scope);
		}
		else if (CHECK_NODE_TYPE(block, parser::NodeBlock, node)) {
			execute_node_block(block);
		}
	}

	void CryoThread::execute_scope_node(const parser::ScopeNode* scope) {
		m_Stack.push_scope();
		execute_node_block(scope);
		m_Stack.pop_scope();
	}

	void CryoThread::execute_node_block(const parser::NodeBlock* block) {
		for (const auto& node : block->Block) {
			execute_node(node.get());
		}
	}

	void CryoThread::execute_variable_declaration_node(const parser::VariableDeclarationNode* var) {
		auto result = m_Stack.push_variable(var->VariableIdentifier->Identifier.lexeme,
			get_type_from_string(var->TypeIdentifier->Identifier.lexeme));
		if (result.has_value()) {
			// TODO: handle error
		}
	}

}