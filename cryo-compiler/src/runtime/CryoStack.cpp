#include "cryopch.h"
#include "CryoStack.h"

#include "TypeID.h"

namespace cryo::runtime {

	CryoStack::CryoStack(uint64_t stack_size) 
		: m_Buffer(stack_size) {
		m_CallStack.push(CallStackEntry());
	}

	CryoStack::~CryoStack() {
	}

	std::optional<push_error> CryoStack::push_variable(const std::string& name, TypeID type) {
		if (type == VOID) {
			return InvalidType;
		}

		if (m_CallStack.top().Variables.contains(name)) {
			return NameAlredyUsed;
		}

		uint32_t size = get_type_size(type);
		uint32_t location = m_StackCounter;

		m_StackCounter += size;
		if (m_StackCounter >= m_Buffer.size()) {
			return StackOverflow;
		}

		m_Variables.emplace_back(VariableData{ type, name, location });
		m_CallStack.top().Variables.insert(std::make_pair(name, m_Variables.size() - 1));
		m_CallStack.top().Scopes.top()++;

		return {};
	}

	void CryoStack::push_function_call() {
		m_CallStack.push(CallStackEntry());
	}

	bool CryoStack::pop_function_call() {
		if (m_CallStack.size() == 1) {
			return false;
		}

		m_CallStack.pop();
		return true;
	}

	void CryoStack::push_scope() {
		m_CallStack.top().Scopes.push(0);
	}

	bool CryoStack::pop_scope() {
		if (m_CallStack.top().Scopes.size() == 1) {
			return false;
		}

		uint32_t count = m_CallStack.top().Scopes.top();
		if (m_Variables.size() < count) {
			return false;
		}
		m_Variables.erase(m_Variables.end() - count, m_Variables.end());

		return true;
	}

	CryoStack::CallStackEntry::CallStackEntry() {
		Scopes.push(0);
	}
}