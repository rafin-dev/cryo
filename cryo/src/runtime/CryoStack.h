#pragma once

#include "TypeID.h"
#include "parser/AST.h"

#include <stack>
#include <expected>
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

		std::expected<ExpressionResult*, push_error> push_variable(const std::string& name);

		void push_function_call();
		bool pop_function_call();

		void push_scope();
		bool pop_scope();

		ExpressionResult* get_variable(const std::string& name);

		struct VariableData {
			std::string Name;
			uint32_t Location = 0;
		};

		const VariableData* get_var_data(const std::string& name);

	private:
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

}