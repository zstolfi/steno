#include "window.hh"
#include <vector>
#include <tuple>
#include <span>
#include <memory>
#include <cstdio>
#include <cstdint>



struct State {
	static bool dragOver;
//	static std::optional<float> transferProgress;
//	static std::unique_ptr<File> files;
	bool running = true;
};

extern "C" { // These functions are called from the browser.
	bool State::dragOver = false;

	void setDragOver(bool input) { State::dragOver = input; }
	void receiveFile(char const* name, std::size_t size, uint8_t const* data) {
		std::span bytes {data, size};
		std::printf("File: (%s) received in %zu bytes\n", name, size);
		for (uint8_t c : bytes) putchar(c);
		if (bytes.back() != '\n') putchar('\n');
	}
}



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

	ImVec4 color = {0.10, 0.10, 0.11, 1.0};
	if (state.dragOver) color.x += 0.3;

	window.render(color);
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
