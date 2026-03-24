#include "raylib_bindings.h"

#include <raylib.h>

namespace cryo::runtime::raylib {

	CryoValue rl_init_window(const std::vector<CryoValue>& params)
	{
		InitWindow(
			std::get<int64_t>(params[0]), 
			std::get<int64_t>(params[1]),
			std::get<std::string>(params[2]).c_str());
		return CryoValue();
	}

	CryoValue rl_close_window(const std::vector<CryoValue>& params)
	{
		CloseWindow();
		return CryoValue();
	}

	CryoValue rl_window_should_close(const std::vector<CryoValue>& params)
	{
		return WindowShouldClose();
	}

	CryoValue rl_begin_drawing(const std::vector<CryoValue>& params)
	{
		BeginDrawing();
		return CryoValue();
	}

	CryoValue rl_end_drawing(const std::vector<CryoValue>& params)
	{
		EndDrawing();
		return CryoValue();
	}
}