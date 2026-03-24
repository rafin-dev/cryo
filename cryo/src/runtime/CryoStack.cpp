#include "cryopch.h"
#include "CryoStack.h"

#include "TypeID.h"

namespace cryo::runtime {

	CryoStack::CryoStack(uint64_t stack_size) 
		: m_Buffer(stack_size) {
	}

	CryoStack::~CryoStack() {
	}

	std::expected<ExpressionResult*, push_error> CryoStack::push_variable(const std::string& name) {
		if (m_CallStack.top().Variables.contains(name)) {
			return std::unexpected(NameAlredyUsed);
		}

		uint32_t location = m_StackCounter;

		m_StackCounter += sizeof(ExpressionResult);
		if (m_StackCounter >= m_Buffer.size()) {
			return std::unexpected(StackOverflow);
		}

		m_Variables.emplace_back(VariableData{ name, location });
		m_CallStack.top().Variables.insert(std::make_pair(name, m_Variables.size() - 1));
		m_CallStack.top().Scopes.top()++;

		return reinterpret_cast<ExpressionResult*>(m_Buffer.data() + location);
	}

	void CryoStack::push_function_call() {
		m_CallStack.push(CallStackEntry());
		push_scope();
	}

	bool CryoStack::pop_function_call() {
		if (m_CallStack.size() == 1) {
			return false;
		}

		while (!m_CallStack.top().Scopes.empty()) {
			pop_scope();
		}

		m_CallStack.pop();
		return true;
	}

	void CryoStack::push_scope() {
		m_CallStack.top().Scopes.push(0);
	}

	bool CryoStack::pop_scope() {
		if (m_CallStack.top().Scopes.empty()) {
			return false;
		}

		uint32_t count = m_CallStack.top().Scopes.top();
		if (m_Variables.size() < count) {
			return false;
		}
		for (uint32_t i = 0; i < count; i++) {
			m_CallStack.top().Variables.erase(m_Variables[m_Variables.size() - 1 - i].Name);
		}
		m_Variables.erase(m_Variables.end() - count, m_Variables.end());
		m_CallStack.top().Scopes.pop();

		return true;
	}

	ExpressionResult* CryoStack::get_variable(const std::string& name) {
		auto ite = m_CallStack.top().Variables.find(name);
		if (ite == m_CallStack.top().Variables.end()) {
			return nullptr;
		}

		auto& var_data = m_Variables[ite->second];

		return (ExpressionResult*)(m_Buffer.data() + var_data.Location);
	}

	const CryoStack::VariableData* CryoStack::get_var_data(const std::string& name)
	{
		auto ite = m_CallStack.top().Variables.find(name);
		if (ite == m_CallStack.top().Variables.end()) {
			return nullptr;
		}

		return &m_Variables[ite->second];
	}
}