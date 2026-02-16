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
		m_Stack.push_function_call();
		execute_node_block(m_Function->Body.get());
	}

#define CHECK_NODE_TYPE(var, Type, node) const auto* var = dynamic_cast<const Type*>(node); var != nullptr

	void CryoThread::execute_node(const parser::Node* node) {
		if (CHECK_NODE_TYPE(var, parser::VariableDeclarationNode, node)) {
			execute_variable_declaration_node(var);
		}
		else if (CHECK_NODE_TYPE(ass, parser::AssignmentOperation, node)) {
			execute_assignment_operation_node(ass);
		}
		else if (CHECK_NODE_TYPE(scope, parser::ScopeNode, node)) {
			execute_scope_node(scope);
		}
		else if (CHECK_NODE_TYPE(block, parser::NodeBlock, node)) {
			execute_node_block(block);
		}
		else if (CHECK_NODE_TYPE(print, parser::PrintNode, node)) {
			execute_print_node(print);
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
		TypeID type = VOID;
		{
			auto result = get_type_from_string(var->TypeIdentifier->Identifier.lexeme);
			if (!result.has_value()) {
				// TODO: user defined type
			}
			else {
				type = result.value();
			}
		}

		auto result = m_Stack.push_variable(var->VariableIdentifier->Identifier.lexeme, type);
		if (result.has_value()) {
			switch (result.value()) {
			case StackOverflow:
				throw std::runtime_error("StackOverflow");

			case NameAlredyUsed:
				throw std::runtime_error("Cryo does not support variable redefinitions!");

			case InvalidType:
				throw std::runtime_error("Type is either invalid or unknown!");
			}
		}
	}

#define GET_IF(ass_type, var, T) if (auto var = std::get_if<T>(&value)) { succes = true; \
 *m_Stack.get_variable_as_cpp_type<ass_type>(var_data->Name) = *var; break; }

	void CryoThread::execute_assignment_operation_node(const parser::AssignmentOperation* ass) {
		auto var_data = m_Stack.get_var_data(ass->LeftValue->Identifier.lexeme);
		if (var_data == nullptr) {
			throw std::runtime_error("Assignment for an unknwon variable");
		}

		auto expr_result = evaluate_expression(ass->RightValue.get());
		if (!expr_result.has_value()) {
			throw std::runtime_error("Failed to evaluate expression!");
		}

		auto value = std::move(expr_result.value());
		bool succes = false;
		switch (var_data->Type) {
		case BOOL: {
			GET_IF(bool, b, bool);
			break;
		}
		
		case CHAR: {
			GET_IF(char, c, char);

			GET_IF(bool, u8, uint8_t);
			GET_IF(bool, u16, uint16_t);
			GET_IF(bool, u32, uint32_t);
			GET_IF(bool, u64, uint64_t);
				   
			GET_IF(bool, i8, int8_t);
			GET_IF(bool, i16, int16_t);
			GET_IF(bool, i32, int32_t);
			GET_IF(bool, i64, int64_t);
				   
			GET_IF(bool, c, char);
				   
			GET_IF(bool, f, float);
			GET_IF(bool, d, double);
			break;
		}

		case U8: {
			GET_IF(uint8_t, u8, uint8_t);
			GET_IF(uint8_t, u16, uint16_t);
			GET_IF(uint8_t, u32, uint32_t);
			GET_IF(uint8_t, u64, uint64_t);

			GET_IF(uint8_t, i8, int8_t);
			GET_IF(uint8_t, i16, int16_t);
			GET_IF(uint8_t, i32, int32_t);
			GET_IF(uint8_t, i64, int64_t);

			GET_IF(uint8_t, c, char);

			GET_IF(uint8_t, f, float);
			GET_IF(uint8_t, d, double);
			break;
		}

		case I8: {
			GET_IF(int8_t, u8, uint8_t);
			GET_IF(int8_t, u16, uint16_t);
			GET_IF(int8_t, u32, uint32_t);
			GET_IF(int8_t, u64, uint64_t);

			GET_IF(int8_t, i8, int8_t);
			GET_IF(int8_t, i16, int16_t);
			GET_IF(int8_t, i32, int32_t);
			GET_IF(int8_t, i64, int64_t);

			GET_IF(int8_t, c, char);

			GET_IF(int8_t, f, float);
			GET_IF(int8_t, d, double);
			break;
		}

		case U16: {
			GET_IF(uint16_t, u8, uint8_t);
			GET_IF(uint16_t, u16, uint16_t);
			GET_IF(uint16_t, u32, uint32_t);
			GET_IF(uint16_t, u64, uint64_t);
					   
			GET_IF(uint16_t, i8, int8_t);
			GET_IF(uint16_t, i16, int16_t);
			GET_IF(uint16_t, i32, int32_t);
			GET_IF(uint16_t, i64, int64_t);
					   
			GET_IF(uint16_t, c, char);
					   
			GET_IF(uint16_t, f, float);
			GET_IF(uint16_t, d, double);
			break;
		}

		case I16: {
			GET_IF(int16_t, u8, uint8_t);
			GET_IF(int16_t, u16, uint16_t);
			GET_IF(int16_t, u32, uint32_t);
			GET_IF(int16_t, u64, uint64_t);
				   
			GET_IF(int16_t, i8, int8_t);
			GET_IF(int16_t, i16, int16_t);
			GET_IF(int16_t, i32, int32_t);
			GET_IF(int16_t, i64, int64_t);
				   
			GET_IF(int16_t, c, char);
				   
			GET_IF(int16_t, f, float);
			GET_IF(int16_t, d, double);
			break;
		}

		case U32: {
			GET_IF(uint32_t, u8, uint8_t);
			GET_IF(uint32_t, u16, uint16_t);
			GET_IF(uint32_t, u32, uint32_t);
			GET_IF(uint32_t, u64, uint64_t);
					   
			GET_IF(uint32_t, i8, int8_t);
			GET_IF(uint32_t, i16, int16_t);
			GET_IF(uint32_t, i32, int32_t);
			GET_IF(uint32_t, i64, int64_t);
					   
			GET_IF(uint32_t, c, char);
					   
			GET_IF(uint32_t, f, float);
			GET_IF(uint32_t, d, double);
			break;
		}

		case I32: {
			GET_IF(int32_t, u8, uint8_t);
			GET_IF(int32_t, u16, uint16_t);
			GET_IF(int32_t, u32, uint32_t);
			GET_IF(int32_t, u64, uint64_t);
					  
			GET_IF(int32_t, i8, int8_t);
			GET_IF(int32_t, i16, int16_t);
			GET_IF(int32_t, i32, int32_t);
			GET_IF(int32_t, i64, int64_t);
					  
			GET_IF(int32_t, c, char);
					  
			GET_IF(int32_t, f, float);
			GET_IF(int32_t, d, double);
			break;
		}

		case U64: {
			GET_IF(uint64_t, u8, uint8_t);
			GET_IF(uint64_t, u16, uint16_t);
			GET_IF(uint64_t, u32, uint32_t);
			GET_IF(uint64_t, u64, uint64_t);
					   
			GET_IF(uint64_t, i8, int8_t);
			GET_IF(uint64_t, i16, int16_t);
			GET_IF(uint64_t, i32, int32_t);
			GET_IF(uint64_t, i64, int64_t);
					   
			GET_IF(uint64_t, c, char);
					   
			GET_IF(uint64_t, f, float);
			GET_IF(uint64_t, d, double);
			break;
		}

		case I64: {
			GET_IF(int64_t, u8, uint8_t);
			GET_IF(int64_t, u16, uint16_t);
			GET_IF(int64_t, u32, uint32_t);
			GET_IF(int64_t, u64, uint64_t);
					  
			GET_IF(int64_t, i8, int8_t);
			GET_IF(int64_t, i16, int16_t);
			GET_IF(int64_t, i32, int32_t);
			GET_IF(int64_t, i64, int64_t);
					  
			GET_IF(int64_t, c, char);
					  
			GET_IF(int64_t, f, float);
			GET_IF(int64_t, d, double);
			break;
		}

		case F32: {
			GET_IF(float, u8, uint8_t);
			GET_IF(float, u16, uint16_t);
			GET_IF(float, u32, uint32_t);
			GET_IF(float, u64, uint64_t);
				   
			GET_IF(float, i8, int8_t);
			GET_IF(float, i16, int16_t);
			GET_IF(float, i32, int32_t);
			GET_IF(float, i64, int64_t);
				   
			GET_IF(float, c, char);
				   
			GET_IF(float, f, float);
			GET_IF(float, d, double);
			break;
		}

		case F64: {
			GET_IF(double, u8, uint8_t);
			GET_IF(double, u16, uint16_t);
			GET_IF(double, u32, uint32_t);
			GET_IF(double, u64, uint64_t);
				   
			GET_IF(double, i8, int8_t);
			GET_IF(double, i16, int16_t);
			GET_IF(double, i32, int32_t);
			GET_IF(double, i64, int64_t);
				   
			GET_IF(double, c, char);
				   
			GET_IF(double, f, float);
			GET_IF(double, d, double);
			break;
		}
		}
		if (!succes) {
			throw std::runtime_error("Invalid Expression for assignment");
		}
	}

	void CryoThread::execute_print_node(const parser::PrintNode* print) {
		auto var_data = m_Stack.get_var_data(print->Value->Identifier.lexeme);
		if (var_data == nullptr) {
			throw std::runtime_error("Unknown variable!");
		}

		std::cout << "print: ";
		switch (var_data->Type) {
		case I32:
			std::cout << *m_Stack.get_variable_as_cpp_type<uint32_t>(print->Value->Identifier.lexeme);
			break;

		case F32:
			std::cout << *m_Stack.get_variable_as_cpp_type<float>(print->Value->Identifier.lexeme);
			break;
		}
		std::cout << std::endl;
	}

	std::optional<ExpressionResult> CryoThread::evaluate_expression(const parser::Node* node)
	{
		if (CHECK_NODE_TYPE(lit, parser::LiteralNode, node)) {
			return evaluate_literal_node(lit);
		}

		return {};
	}

	std::optional<ExpressionResult> CryoThread::evaluate_literal_node(const parser::LiteralNode* node)
	{
		if (CHECK_NODE_TYPE(integer, parser::IntegerLiteralNode, node)) {
			return ExpressionResult(std::stoi(integer->Value.lexeme));
		}
		else if (CHECK_NODE_TYPE(floating, parser::FloatLiteralNode, node)) {
			return ExpressionResult(std::stof(floating->Value.lexeme));
		}
		else if (CHECK_NODE_TYPE(boolean, parser::BoolLiteralNode, node)) {
			return ExpressionResult(boolean->Value);
		}
		else if (CHECK_NODE_TYPE(character, parser::CharLiteralNode, node)) {
			return ExpressionResult(character->Character);
		}
		else if (CHECK_NODE_TYPE(string, parser::StringLiteralNode, node)) {
			return ExpressionResult(string->Value.lexeme);
		}

		return std::optional<ExpressionResult>();
	}

}