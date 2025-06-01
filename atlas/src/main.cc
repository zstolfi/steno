#include "imgui.h"
#include <emscripten.h>

#include "window.hh"
#include <tuple>
#include <cstdio>



struct State {
	/* State Goes Here */
	bool running = true;
};

void mainLoop(Window& window, State& state, ImGuiIO& io) {
#ifdef __EMSCRIPTEN__
	if (!state.running) emscripten_cancel_main_loop();
#endif
	
	for (SDL_Event e; SDL_PollEvent(&e);) {
		ImGui_ImplSDL3_ProcessEvent(&e);
		if (e.type == SDL_EVENT_QUIT) state.running = false;
	}

	window.newFrame();

	/* GUI Goes Here */

	window.render({0.10, 0.10, 0.11, 1.0});
}



int main(int argc, char const* argv[]) {
	Window window {"Steno Atlas", 1280, 720};
	State state {};

#ifdef __EMSCRIPTEN__
	auto userData = std::tie(window, state, ImGui::GetIO());
	emscripten_set_main_loop_arg(
		[] (void* data) { std::apply(mainLoop, *(decltype(userData)*)data); },
		&userData, 0, true
	);
#else
	while (state.running) mainLoop(window, state, ImGui::GetIO());
#endif

}
