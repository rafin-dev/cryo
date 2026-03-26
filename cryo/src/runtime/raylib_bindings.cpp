#include "raylib_bindings.h"

#include <raylib.h>

namespace cryo::runtime::raylib {

	CryoValue rl_init_window(const std::vector<CryoValue>& params)
	{
		InitWindow(
			std::get<int64_t>(params[0]), 
			std::get<int64_t>(params[1]),
			std::get<std::string>(params[2]).c_str());
		SetTargetFPS(60);
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

	CryoValue rl_clear_background(const std::vector<CryoValue>& params)
	{
		ClearBackground(
			Color{
				static_cast<unsigned char>(std::get<int64_t>(params[0])),
				static_cast<unsigned char>(std::get<int64_t>(params[1])),
				static_cast<unsigned char>(std::get<int64_t>(params[2])),
				static_cast<unsigned char>(std::get<int64_t>(params[3]))
			}
		);
		return CryoValue();
	}

	CryoValue rl_draw_rectangle(const std::vector<CryoValue>& params)
	{
		DrawRectangle(
			std::get<int64_t>(params[0]),
			std::get<int64_t>(params[1]),
			std::get<int64_t>(params[2]),
			std::get<int64_t>(params[3]),
			Color{
				static_cast<unsigned char>(std::get<int64_t>(params[4])),
				static_cast<unsigned char>(std::get<int64_t>(params[5])),
				static_cast<unsigned char>(std::get<int64_t>(params[6])),
				static_cast<unsigned char>(std::get<int64_t>(params[7]))
			}
		);

		return CryoValue();
	}

	CryoValue rl_is_space_pressed(const std::vector<CryoValue>&) {
		return IsKeyDown(KEY_SPACE);
	}
}