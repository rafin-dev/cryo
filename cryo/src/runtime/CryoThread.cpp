#include "cryopch.h"
#include "CryoThread.h"

#include "CryoContext.h"
#include "TypeID.h"

namespace cryo::runtime {

	CryoThread::CryoThread(const parser::FunctionDefinitionNode* func, const CryoContext* context) 
		: m_Context(context), m_Function(func) {
	}

	CryoThread::~CryoThread() {
	}

	void CryoThread::run(const std::vector<CryoValue>& param) {
		m_Stack.push_function_call();
		execute_node_block(m_Function->Body.get());
		if (m_Flag != None && m_Flag != Return) {
			throw std::runtime_error("breaks and continues need to be called from inside a loop!");
		}
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
		else if (CHECK_NODE_TYPE(ite, parser::IfThenElseNode, node)) {
			execute_if_then_else_node(ite);
		}
		else if (CHECK_NODE_TYPE(wl, parser::WhileNode, node)) {
			execute_while_loop(wl);
		}
		else if (CHECK_NODE_TYPE(ln, parser::LoopNode, node)) {
			execute_loop(ln);
		}
		else if (CHECK_NODE_TYPE(rt, parser::ReturnStatementNode, node)) {
			execute_return_node(rt);
		}
		else if (CHECK_NODE_TYPE(func, parser::FunctionCallNode, node)) {
			evaluate_function_call_node(func);
		}
		else if (CHECK_NODE_TYPE(ct, parser::ContinueStatementNode, node)) {
			m_Flag = Continue;
		}
		else if (CHECK_NODE_TYPE(ct, parser::BreakStatementNode, node)) {
			m_Flag = Break;
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

			if (m_Flag != None) {
				return;
			}
		}
	}

	void CryoThread::execute_variable_declaration_node(const parser::VariableDeclarationNode* var) {
		auto result = m_Stack.push_variable(var->Identifier);
		if (!result.has_value()) {
			switch (result.error()) {
			case StackOverflow:
				throw std::runtime_error("StackOverflow");

			case NameAlredyUsed:
				throw std::runtime_error("Cryo does not support variable redefinitions!");

			case InvalidType:
				throw std::runtime_error("Type is either invalid or unknown!");
			}
		}
	}

	template<typename OP_int, typename OP_float>
	void assign_and_op(CryoValue* left, CryoValue* right) {
		OP_int int_op;
		OP_float float_op;
		if (int64_t* left_int = std::get_if<int64_t>(left)) {
			if (int64_t* right_int = std::get_if<int64_t>(right)) {
				*left_int = int_op(*left_int, *right_int);
			}
			else if (double* right_float = std::get_if<double>(right)) {
				*left_int = float_op(*left_int, *right_float);
			}
		}
		else if (double* left_float = std::get_if<double>(left)) {
			if (int64_t* right_int = std::get_if<int64_t>(right)) {
				*left_float = float_op(*left_float, *right_int);
			}
			else if (double* right_float = std::get_if<double>(right)) {
				*left_float = float_op(*left_float, *right_float);
			}
		}
	}

	void CryoThread::execute_assignment_operation_node(const parser::AssignmentOperation* ass) {
		auto expr_result = evaluate_expression(ass->RightValue.get());
		if (!expr_result.has_value()) {
			throw std::runtime_error("Failed to evaluate expression!");
		}

		// Variable data needs to be retrieved after the expression is resolved
		// Because expression may include function calls
		// Which could make the m_Stack object perform realocations for it's variable data vector
		// Invalidating the pointer to the variable data
		CryoValue* var = nullptr;
		// Assignment directly to a variable
		if (auto identifier = dynamic_cast<const parser::IdentifierNode*>(ass->LeftValue.get())) {
			var = m_Stack.get_variable(identifier->Identifier);
		}
		// TODO: other kinds of assignments
		if (var == nullptr) {
			throw std::runtime_error("Assignment for an unknwon variable");
		}

		auto right = std::move(expr_result.value());
		switch (ass->Operator) {
			case parser::TokenType::EQUAL: {
				*var = right;
				break;
			}
			
			case parser::TokenType::PLUS_EQUAL: {
				assign_and_op<std::plus<int64_t>, std::plus<double>>(var, &right);
				break;
			}

			case parser::TokenType::MINUS_EQUAL: {
				assign_and_op<std::minus<int64_t>, std::minus<double>>(var, &right);
				break;
			}

			case parser::TokenType::SLASH_EQUAL: {
				assign_and_op<std::divides<int64_t>, std::divides<double>>(var, &right);
				break;
			}

			case parser::TokenType::ASTERISK_EQUAL: {
				assign_and_op<std::multiplies<int64_t>, std::multiplies<double>>(var, &right);
				break;
			}

			default:
				// This really shouldn't ever happen
				throw std::runtime_error("Something went wrong when parsing!");
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
		bool running = true;
		while (running) {
			auto result = evaluate_condition(wl->Condition.get());

			if (!result.has_value()) {
				throw std::runtime_error("Failed to evaluate condition!");
			}
			if (!result.value()) {
				running = false;
			}

			execute_node(wl->Body.get());

			switch (m_Flag) {
			case Continue:
				m_Flag = None;
				break;

			case Break:
				m_Flag = None;
			case Return:
				running = false;
			}
		}
	}

	void CryoThread::execute_loop(const parser::LoopNode* ln) {
		uint64_t count = 0;
		uint64_t i = 0;

		if (ln->Count != nullptr) {
			auto expr = evaluate_expression(ln->Count.get());
			if (!expr.has_value()) {
				throw std::runtime_error("invalid loop count expression!");
			}
			count = get_loop_count(expr.value());
		}
		
		while (ln->Count == nullptr || i < count) {
			execute_node(ln->Body.get());
			i++;

			switch (m_Flag) {
			case Continue:
				m_Flag = None;
				break;

			case Break:
				m_Flag = None;
			case Return:
				break;
			}
		}
	}

	void CryoThread::execute_return_node(const parser::ReturnStatementNode* return_statement) {
		m_Flag = Return;
		if (!return_statement->ReturnValue) {
			return;
		}

		auto result = evaluate_expression(return_statement->ReturnValue.get());
		if (!result.has_value()) {
			throw std::runtime_error("Failed to evaluate expression!");
		}

		m_ReturnHasValue = true;
		m_ReturnValue = std::move(result.value());
	}

	std::optional<CryoValue> CryoThread::evaluate_function_call_node(const parser::FunctionCallNode* func_call) {
		auto func = m_Context->get_function(func_call->FuncID->Identifier);
		const InternalFunction* internal_func = nullptr;
		if (func == nullptr) {
			internal_func = m_Context->get_internal_function(func_call->FuncID->Identifier);
			if (internal_func == nullptr) {
				throw std::runtime_error("Function does not exist!");
			}
		}
		else if (func_call->Arguments->Block.size() != func->Parameters.size()) {
			throw std::runtime_error("Wrong number of arguments!");
		}
		
		// Resolve parameters expressions
		std::vector<CryoValue> params;
		params.reserve(func_call->Arguments->Block.size());
		for (auto& expr : func_call->Arguments->Block) {
			auto result = evaluate_expression(expr.get());
			if (!result.has_value()) {
				throw std::runtime_error("Failed to evaluate parameter!");
			}
			params.push_back(std::move(result.value()));
		}

		if (func) {
			m_Stack.push_function_call();

			// Create parameter variables
			for (int i = 0; i < params.size(); i++) {
				auto var = m_Stack.push_variable(func->Parameters[i]->Identifier);
				if (!var.has_value()) {
					throw std::runtime_error("Failed to create parameter variable!");
				}

				auto value = evaluate_expression(func_call->Arguments->Block[i].get());
				if (!value.has_value()) {
					throw std::runtime_error("Failed to evaluate argument!");
				}

				// This looks kinda funny
				*var.value() = value.value();
			}

			execute_node_block(func->Body.get());

			m_Stack.pop_function_call();

			m_Flag = None;
			if (m_ReturnHasValue) {
				m_ReturnHasValue = false;
				return m_ReturnValue;
			}
			return {};
		}
		else {
			return (*internal_func)(params);
		}
	}

	uint64_t CryoThread::get_loop_count(CryoValue result)
	{
		if (auto i64 = std::get_if<int64_t>(&result)) {
			if (*i64 < 0) {
				throw std::runtime_error("Loop count cannot be a negative value!");
			}
			return *i64;
		}

		throw std::runtime_error("loop condition needs to be an integer!");
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

	std::optional<CryoValue> CryoThread::evaluate_expression(const parser::Node* node)
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
		if (CHECK_NODE_TYPE(func_call, parser::FunctionCallNode, node)) {
			return evaluate_function_call_node(func_call);
		}

		return {};
	}

	std::optional<CryoValue> CryoThread::evaluate_literal_node(const parser::LiteralNode* node)
	{
		if (CHECK_NODE_TYPE(integer, parser::IntegerLiteralNode, node)) {
			return CryoValue(std::stoi(integer->Value));
		}
		else if (CHECK_NODE_TYPE(floating, parser::FloatLiteralNode, node)) {
			return CryoValue(std::stof(floating->Value));
		}
		else if (CHECK_NODE_TYPE(boolean, parser::BoolLiteralNode, node)) {
			return CryoValue(boolean->Value);
		}
		else if (CHECK_NODE_TYPE(character, parser::CharLiteralNode, node)) {
			return CryoValue(character->Character);
		}
		else if (CHECK_NODE_TYPE(string, parser::StringLiteralNode, node)) {
			return CryoValue(string->Value);
		}

		return {};
	}

	std::optional<CryoValue> CryoThread::evaluate_identifier_node(const parser::IdentifierNode* node)
	{
		auto var = m_Stack.get_variable(node->Identifier);
		if (var == nullptr) {
			return {};
		}

		return *var;
	}


#define CHECK_AND_EVALUATE(left, op, right_type, r) if (auto r = std::get_if<right_type>(&right)) { \
return op((*left), (*r)); }

	template<typename LEFT, typename op>
	std::optional<CryoValue> operate_numeric_value(const CryoValue& left, const CryoValue& right) {
		op operation;
		if (auto left_value = std::get_if<LEFT>(&left)) {
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
	OPERATE_NUMERIC(int64_t, i64, op);	\
	OPERATE_NUMERIC_F(double, double, f64, op);	\
	break;										\
	}

	std::optional<CryoValue> CryoThread::evaluate_binary_operation(const parser::BinaryOperation* op)
	{
		auto left_result = evaluate_expression(op->LeftValue.get());
		auto& operation = op->Operator;
		auto right_result = evaluate_expression(op->RightValue.get());
		if (!left_result.has_value() || !right_result.has_value()) {
			return {};
		}

		auto left_value = std::move(left_result.value());
		auto right_value = std::move(right_result.value());

		// TODO: divide by 0 exception
		switch (operation) {
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

	std::optional<CryoValue> CryoThread::evaluate_unary_operation(const parser::UnaryOperation* op)
	{
		auto v_result = evaluate_expression(op->Value.get());
		auto& operation = op->Operator;
		if (!v_result.has_value()) {
			return {};
		}

		auto value = std::move(v_result.value());

		switch (operation) {
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