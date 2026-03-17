#pragma once

#include "TypeID.h"

namespace cryo::runtime::raylib {

	ExpressionResult rl_init_window(const std::vector<ExpressionResult>& params);
	ExpressionResult rl_close_window(const std::vector<ExpressionResult>& params);

	ExpressionResult rl_window_should_close(const std::vector<ExpressionResult>& params);

	ExpressionResult rl_begin_drawing(const std::vector<ExpressionResult>& params);
	ExpressionResult rl_end_drawing(const std::vector<ExpressionResult>& params);
}