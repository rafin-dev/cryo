#pragma once

#include "TypeID.h"

namespace cryo::runtime::raylib {

	CryoValue rl_init_window(const std::vector<CryoValue>& params);
	CryoValue rl_close_window(const std::vector<CryoValue>& params);

	CryoValue rl_window_should_close(const std::vector<CryoValue>& params);

	CryoValue rl_begin_drawing(const std::vector<CryoValue>& params);
	CryoValue rl_end_drawing(const std::vector<CryoValue>& params);

	CryoValue rl_clear_background(const std::vector<CryoValue>& params);
	CryoValue rl_draw_rectangle(const std::vector<CryoValue>& params);

	CryoValue rl_is_space_pressed(const std::vector<CryoValue>&);
}