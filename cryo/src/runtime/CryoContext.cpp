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

		set_internal_function("println", [](const std::vector<CryoValue>& params) -> CryoValue {
			for (auto& param : params) {
				if (auto str = std::get_if<std::string>(&param)) {
					std::cout << *str;
				}
				else if (auto integer = std::get_if<int64_t>(&param)) {
					std::cout << *integer;
				}
				else if (auto floating = std::get_if<double>(&param)) {
					std::cout << *floating;
				}
			}
			std::cout << std::endl;
			return CryoValue();
			});
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

		m_MainThread = std::make_unique<CryoThread>(ite->second, this);
		m_MainThread->run();
	}

	const parser::FunctionDefinitionNode* CryoContext::get_function(const std::string& name) const {
		auto ite = m_Functions.find(name);
		if (ite == m_Functions.end()) {
			return nullptr;
		}
		return ite->second;
	}

	const InternalFunction* CryoContext::get_internal_function(const std::string& name) const {
		auto ite = m_InternalFunctions.find(name);
		if (ite == m_InternalFunctions.end()) {
			return nullptr;
		}
		return &ite->second;
	}

	void CryoContext::set_internal_function(const std::string& name, const InternalFunction& func) {
		m_InternalFunctions.insert(std::pair(name, func));
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