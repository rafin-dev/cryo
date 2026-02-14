#pragma once

#include "parser/AST.h"
#include "CryoThread.h"

#include <optional>
#include <filesystem>

namespace cryo::runtime {

	class CryoContext {
	public:
		CryoContext(std::filesystem::path file);
		~CryoContext();

		void run(const std::string& func);

	private:
		void search_NodeBlock(const parser::NodeBlock* block);

		std::filesystem::path m_MainFile;

		std::unique_ptr<CryoThread> m_MainThread;

		std::unique_ptr<parser::NodeBlock> m_SyntaxTree;
		std::unordered_map<std::string, const parser::FunctionDefinitionNode*> m_Functions;
	};

}