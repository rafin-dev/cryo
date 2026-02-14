#include "cryopch.h"
#include "CryoContext.h"

#include "parser/Parser.h"

namespace cryo::runtime {

	CryoContext::CryoContext(std::filesystem::path file) 
		: m_MainFile(std::move(file)) {
		auto result = parser::Parser(m_MainFile, "").parse();
		if (!result.has_value()) {
			std::exit(-1);
		}

		m_SyntaxTree = std::move(result.value());

		search_NodeBlock(m_SyntaxTree.get());
		m_Functions;
	}

	CryoContext::~CryoContext() {
	}

	void CryoContext::run(const std::string& func) {
		m_MainThread.reset();

		auto ite = m_Functions.find(func);
		if (ite == m_Functions.end()) {
			std::cerr << "Function " << func << " does not exist!" << std::endl;
			std::exit(-1);
		}

		m_MainThread = std::make_unique<CryoThread>(ite->second);
		m_MainThread->run();
	}

	void CryoContext::search_NodeBlock(const cryo::parser::NodeBlock* block)
	{
		for (auto& node : block->Block) {
			if (auto* b = dynamic_cast<const parser::NodeBlock*>(node.get()); b != nullptr) {
				search_NodeBlock(b);
			}
			if (auto* func = dynamic_cast<const parser::FunctionDefinitionNode*>(node.get()); func != nullptr) {
				m_Functions.insert(std::pair(func->Identifier->Identifier.lexeme, func));
			}
		}
	}

}