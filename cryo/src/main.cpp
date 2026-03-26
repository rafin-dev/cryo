#include "cryopch.h"

#include "parser/AstBuilder.h"
#include "parser/parser.h"
#include "parser/Lexer.h"
#include "runtime/CryoContext.h"

#include <iostream>
#include <string>
#include <print>

#include "runtime/raylib_bindings.h"

int main(const int argc, const char ** argv) {
    if (argc == 1) {
        std::cerr << "Missing source file" << std::endl;
        return -1;
    }

    std::filesystem::path file(argv[1]);
    if (!std::filesystem::exists(file) || !std::filesystem::is_regular_file(file)) {
        std::cerr << "File at [" << file.string() << "] either doesn`t exist or is not a file!" << std::endl;
        return -1;
    }

    cryo::runtime::CryoContext context(argv[1]);

    // Raylib bindings
    context.set_internal_function("rlInitWindow", cryo::runtime::raylib::rl_init_window);
    context.set_internal_function("rlCloseWindow", cryo::runtime::raylib::rl_close_window);
    context.set_internal_function("rlWindowShouldClose", cryo::runtime::raylib::rl_window_should_close);
    context.set_internal_function("rlBeginDrawing", cryo::runtime::raylib::rl_begin_drawing);
    context.set_internal_function("rlEndDrawing", cryo::runtime::raylib::rl_end_drawing);
    context.set_internal_function("rlClearBackground", cryo::runtime::raylib::rl_clear_background);
    context.set_internal_function("rlDrawRectangle", cryo::runtime::raylib::rl_draw_rectangle);
    context.set_internal_function("rlIsSpaceDown", cryo::runtime::raylib::rl_is_space_pressed);

    context.run("main");
}
