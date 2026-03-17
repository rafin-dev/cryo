#include "raylib_bindings.h"

#include <raylib.h>

namespace cryo::runtime::raylib {

	ExpressionResult rl_init_window(const std::vector<ExpressionResult>& params)
	{
		InitWindow(
			std::get<int32_t>(params[0]), 
			std::get<int32_t>(params[1]),
			std::get<std::string>(params[2]).c_str());
		return ExpressionResult();
	}

	ExpressionResult rl_close_window(const std::vector<ExpressionResult>& params)
	{
		CloseWindow();
		return ExpressionResult();
	}

	ExpressionResult rl_window_should_close(const std::vector<ExpressionResult>& params)
	{
		return WindowShouldClose();
	}

	ExpressionResult rl_begin_drawing(const std::vector<ExpressionResult>& params)
	{
		BeginDrawing();
		return ExpressionResult();
	}

	ExpressionResult rl_end_drawing(const std::vector<ExpressionResult>& params)
	{
		EndDrawing();
		return ExpressionResult();
	}
}