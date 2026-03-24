#pragma once

#include "parser/AST.h"
#include "CryoThread.h"

#include <optional>
#include <filesystem>

namespace cryo::runtime {

	using InternalFunction = std::function<CryoValue(const std::vector<CryoValue>&)>;

	class CryoContext {
	public:
		CryoContext(std::filesystem::path file);
		~CryoContext();

		void run(const std::string& func);

		const parser::FunctionDefinitionNode* get_function(const std::string& name) const;
		const InternalFunction* get_internal_function(const std::string& name) const;

		void set_internal_function(const std::string& name, const InternalFunction& func);

	private:
		void search_NodeBlock(const parser::NodeBlock* block);

		std::filesystem::path m_MainFile;

		std::unique_ptr<CryoThread> m_MainThread;

		std::unique_ptr<parser::NodeBlock> m_SyntaxTree;
		std::unordered_map<std::string, const parser::FunctionDefinitionNode*> m_Functions;
		std::unordered_map<std::string, InternalFunction> m_InternalFunctions;
	};

}