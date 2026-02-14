#pragma once

#include "TypeID.h"

#include <stack>
#include <vector>

namespace cryo::runtime {

	enum push_error {
		InvalidType,
		StackOverflow,
		NameAlredyUsed
	};

	class CryoStack {
	public:
		CryoStack(uint64_t stack_size = 8000000);
		~CryoStack();

		std::optional<push_error> push_variable(const std::string& name, TypeID type);

		void push_function_call(); // TODO: parameters, rerturn type, call stack...
		bool pop_function_call();

		void push_scope();
		bool pop_scope();

		template<typename T>
		T* get_variable_as_cpp_type(const std::string& name);

	private:
		struct VariableData {
			TypeID Type = VOID;
			std::string Name;
			uint32_t Location = 0;
			uint32_t Size = 0;
		};

		struct CallStackEntry {
			CallStackEntry() = default;

			std::unordered_map<std::string, uint32_t> Variables;
			std::stack<uint32_t> Scopes;
		};
		
		std::stack<CallStackEntry> m_CallStack;
		std::vector<uint8_t> m_Buffer;
		std::vector<VariableData> m_Variables;
		uint64_t m_StackCounter = 0;
	};

	template<typename T>
	inline T* CryoStack::get_variable_as_cpp_type(const std::string& name) {
		auto ite = m_CallStack.top().Variables.find(name);
		if (ite == m_CallStack.top().Variables.end()) {
			return nullptr;
		}

		auto& var_data = m_Variables[ite->second];
		if (var_data.Size != sizeof(T)) {
			return nullptr;
		}

		return static_cast<T*>(m_Buffer.data() + var_data.Location);
	}

}