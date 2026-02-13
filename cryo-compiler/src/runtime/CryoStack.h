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

	private:
		struct VariableData {
			TypeID Type = VOID;
			std::string Name;
			uint32_t Location = 0;
		};

		struct CallStackEntry {
			CallStackEntry();

			std::unordered_map<std::string, uint32_t> Variables;
			std::stack<uint32_t> Scopes;
		};
		
		std::stack<CallStackEntry> m_CallStack;
		std::vector<uint8_t> m_Buffer;
		std::vector<VariableData> m_Variables;
		uint64_t m_StackCounter = 0;
	};

}