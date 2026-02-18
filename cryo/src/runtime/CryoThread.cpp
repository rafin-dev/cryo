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
		else if (CHECK_NODE_TYPE(ite, parser::IfThenElseNode, node)) {
			execute_if_then_else_node(ite);
		}
		else if (CHECK_NODE_TYPE(wl, parser::WhileNode, node)) {
			execute_while_loop(wl);
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
 *m_Stack.get_variable_as_cpp_type<ass_type>(var_data->Location) = *var; break; }

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

			GET_IF(char, u8, uint8_t);
			GET_IF(char, u16, uint16_t);
			GET_IF(char, u32, uint32_t);
			GET_IF(char, u64, uint64_t);
				   
			GET_IF(char, i8, int8_t);
			GET_IF(char, i16, int16_t);
			GET_IF(char, i32, int32_t);
			GET_IF(char, i64, int64_t);
				   
			GET_IF(char, c, char);
				   
			GET_IF(char, f, float);
			GET_IF(char, d, double);
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

	void CryoThread::execute_if_then_else_node(const parser::IfThenElseNode* ite) {
		auto result = evaluate_condition(ite->Condition.get());

		if (!result.has_value()) {
			throw std::runtime_error("Failed to evaluate condition!");
		}

		if (result.value()) {
			execute_node(ite->IF.get());
		}
		else {
			execute_node(ite->ELSE.get());
		}
	}

	void CryoThread::execute_while_loop(const parser::WhileNode* wl) {
		while (true) {
			auto result = evaluate_condition(wl->Condition.get());

			if (!result.has_value()) {
				throw std::runtime_error("Failed to evaluate condition!");
			}
			if (!result.value()) {
				break;
			}

			execute_node(wl->Body.get());
		}
	}

	void CryoThread::execute_print_node(const parser::PrintNode* print) {
		auto result = evaluate_expression(print->Value.get());
		if (!result.has_value()) {
			throw std::runtime_error("Failed to evalueate expression!");
		}
		auto value = std::move(result.value());

		if (auto u32 = std::get_if<uint32_t>(&value)) {
			std::cout << *u32;
		}
		else if (auto i32 = std::get_if<int32_t>(&value)) {
			std::cout << *i32;
		}
		else if (auto f32 = std::get_if<float>(&value)) {
			std::cout << *f32;
		}
		else if (auto b8 = std::get_if<bool>(&value)) {
			std::cout << (*b8 ? "true" : "false");
		}
		else if (auto str = std::get_if<std::string>(&value)) {
			std::cout << *str;
		}
		std::cout << std::endl;
	}

	std::optional<bool> CryoThread::evaluate_condition(const parser::Node* node) {
		auto result = evaluate_expression(node);
		if (!result.has_value()) {
			return {};
		}

		if (auto bptr = std::get_if<bool>(&result.value())) {
			return *bptr;
		}

		return {};
	}

	std::optional<ExpressionResult> CryoThread::evaluate_expression(const parser::Node* node)
	{
		if (CHECK_NODE_TYPE(lit, parser::LiteralNode, node)) {
			return evaluate_literal_node(lit);
		}
		if (CHECK_NODE_TYPE(var, parser::IdentifierNode, node)) {
			return evaluate_identifier_node(var);
		}
		if (CHECK_NODE_TYPE(bin_op, parser::BinaryOperation, node)) {
			return evaluate_binary_operation(bin_op);
		}
		if (CHECK_NODE_TYPE(un_op, parser::UnaryOperation, node)) {
			return evaluate_unary_operation(un_op);
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
			return ExpressionResult(string->Value);
		}

		return {};
	}

	std::optional<ExpressionResult> CryoThread::evaluate_identifier_node(const parser::IdentifierNode* node)
	{
		auto var_data = m_Stack.get_var_data(node->Identifier.lexeme);
		if (!var_data) {
			return {};
		}

		switch (var_data->Type) {
		case BOOL:
			return *m_Stack.get_variable_as_cpp_type<bool>(var_data->Location);
		case CHAR:
			return *m_Stack.get_variable_as_cpp_type<char>(var_data->Location);
		case U8:
			return *m_Stack.get_variable_as_cpp_type<uint8_t>(var_data->Location);
		case I8:
			return *m_Stack.get_variable_as_cpp_type<int8_t>(var_data->Location);

		case U16:
			return *m_Stack.get_variable_as_cpp_type<uint16_t>(var_data->Location);
		case I16:
			return *m_Stack.get_variable_as_cpp_type<int16_t>(var_data->Location);

		case U32:
			return *m_Stack.get_variable_as_cpp_type<uint32_t>(var_data->Location);
		case I32:
			return *m_Stack.get_variable_as_cpp_type<int32_t>(var_data->Location);
		case F32:
			return *m_Stack.get_variable_as_cpp_type<float>(var_data->Location);

		case U64:
			return *m_Stack.get_variable_as_cpp_type<uint64_t>(var_data->Location);
		case I64:
			return *m_Stack.get_variable_as_cpp_type<int64_t>(var_data->Location);
		case F64:
			return *m_Stack.get_variable_as_cpp_type<double>(var_data->Location);
		}
	}


#define CHECK_AND_EVALUATE(left, op, right_type, r) if (auto r = std::get_if<right_type>(&right)) { \
return op((*left), (*r)); }

	template<typename LEFT, typename op>
	std::optional<ExpressionResult> operate_numeric_value(const ExpressionResult& left, const ExpressionResult& right) {
		op operation;
		if (auto left_value = std::get_if<LEFT>(&left)) {
			CHECK_AND_EVALUATE(left_value, operation, uint8_t, ru8);
			CHECK_AND_EVALUATE(left_value, operation, int8_t, ri8);
							   
			CHECK_AND_EVALUATE(left_value, operation, uint16_t, ru16);
			CHECK_AND_EVALUATE(left_value, operation, int16_t, ri16);
							   
			CHECK_AND_EVALUATE(left_value, operation, uint32_t, ru32);
			CHECK_AND_EVALUATE(left_value, operation, int32_t, ri32);
			CHECK_AND_EVALUATE(left_value, operation, float, rf32);
							   
			CHECK_AND_EVALUATE(left_value, operation, uint64_t, ru64);
			CHECK_AND_EVALUATE(left_value, operation, int64_t, ri64);
			CHECK_AND_EVALUATE(left_value, operation, double, rf64);
		}
		return {};
	}

#define OPERATE_NUMERIC_F(LEFT_TYPE, op_type, left, op) if (auto left = operate_numeric_value<LEFT_TYPE, op<op_type>>(left_value, right_value)) { return left; }

#define OPERATE_NUMERIC(LEFT_TYPE, left, op) if (auto left = operate_numeric_value<LEFT_TYPE, op<LEFT_TYPE>>(left_value, right_value)) { return left; }

#define OPERATE_EQUAL(LEFT_TYPE, RIHT_TYPE, left) if (auto left = operate_numeric_value<LEFT_TYPE, std::equal<LEFT_TYPE, RIGHT_TYPE>>(left_value, right_value)) { return left; }


#define SWITCH_LABEL(tkType, op) case tkType: { \
	OPERATE_NUMERIC(bool, b8, op); \
	OPERATE_NUMERIC(char, c8, op); \
	OPERATE_NUMERIC(uint8_t, u8, op);	\
	OPERATE_NUMERIC(int8_t, i8, op);		\
												\
	OPERATE_NUMERIC(uint16_t, u16, op);	\
	OPERATE_NUMERIC(int16_t, i16, op);	\
												\
	OPERATE_NUMERIC(uint32_t, u32, op);	\
	OPERATE_NUMERIC(int32_t, i32, op);	\
	OPERATE_NUMERIC_F(float, float, f32, op);		\
												\
	OPERATE_NUMERIC(uint64_t, u64, op);	\
	OPERATE_NUMERIC(int64_t, i64, op);	\
	OPERATE_NUMERIC_F(double, double, f64, op);	\
	break;										\
	}

	std::optional<ExpressionResult> CryoThread::evaluate_binary_operation(const parser::BinaryOperation* op)
	{
		auto left_result = evaluate_expression(op->LeftValue.get());
		auto& operation = op->Operator;
		auto right_result = evaluate_expression(op->RightValue.get());
		if (!left_result.has_value() || !right_result.has_value()) {
			return {};
		}

		auto left_value = std::move(left_result.value());
		auto right_value = std::move(right_result.value());

		switch (operation.Type) {
			SWITCH_LABEL(parser::TokenType::PLUS, std::plus);
			SWITCH_LABEL(parser::TokenType::MINUS, std::minus);
			SWITCH_LABEL(parser::TokenType::ASTERISK, std::multiplies);
			SWITCH_LABEL(parser::TokenType::SLASH, std::divides);

			SWITCH_LABEL(parser::TokenType::EQUAL_EQUAL, std::equal_to);
			SWITCH_LABEL(parser::TokenType::BANG_EQUAL, std::not_equal_to);
			SWITCH_LABEL(parser::TokenType::LESS, std::less);
			SWITCH_LABEL(parser::TokenType::LESS_EQUAL, std::less_equal);
			SWITCH_LABEL(parser::TokenType::GREATER, std::greater);
			SWITCH_LABEL(parser::TokenType::GREATER_EQUAL, std::greater_equal);
		}

		return {};
	}

	std::optional<ExpressionResult> CryoThread::evaluate_unary_operation(const parser::UnaryOperation* op)
	{
		auto v_result = evaluate_expression(op->Value.get());
		auto& operation = op->Operator;
		if (!v_result.has_value()) {
			return {};
		}

		auto value = std::move(v_result.value());

		switch (operation.Type) {
		case parser::TokenType::BANG: {
			if (auto b = std::get_if<bool>(&value)) {
				return !(*b);
			}

			break;
		}
		}

		return {};
	}

}