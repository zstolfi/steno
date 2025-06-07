#include "window.hh"
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>
#include <tuple>
#include <iterator>



struct File {
	std::string name;
	std::vector<uint8_t> bytes;
	auto operator<=>(File const& f) const { return name <=> f.name; }

	void print() const {
		for (char c : bytes) std::putchar(c);
		if (bytes.back() != '\n') std::putchar('\n');
	}
};

struct Atlas {
	std::vector<uint8_t> image;
	GLuint texture;
	Atlas() : image(256*256*4) {
		for (unsigned y=0; y<256; y++)
		for (unsigned x=0; x<256; x++) {
			std::size_t i = 256*y + x;
			image[4*i + 0] = x^y;
			image[4*i + 1] = x^y;
			image[4*i + 2] = x^y;
			image[4*i + 3] = 255;
		}
		texture = Window::loadTexture(image, 256, 256);
	}
};

namespace State {
	bool running = true;
	bool showDemoWindow = false;

	bool dragOver = false;
//	std::optional<float> transferProgress;
	std::vector<File> files;
	std::map<File, Atlas> atlases;
	Atlas* selectedAtlas = nullptr;
}

extern "C" { // These functions will be called from the browser.
	void setDragOver(bool input) { State::dragOver = input; }
}



#include "gui.hh"

void mainLoop(Window& window, ImGuiIO& io) {
	for (SDL_Event e; SDL_PollEvent(&e);) {
		ImGui_ImplSDL3_ProcessEvent(&e);
		if (e.type == SDL_EVENT_QUIT) State::running = false;
		if (e.type == SDL_EVENT_DROP_FILE) {
			std::filesystem::path path {e.drop.data};
			std::string const name = path.filename();
			std::size_t const size = std::filesystem::file_size(path);
			std::printf("File: (%s) received in %zu bytes\n", name.c_str(), size);
			if (std::ifstream file {path}) {
				State::files.emplace_back(
					name, std::vector<uint8_t> {
						std::istreambuf_iterator<char> {file},
						std::istreambuf_iterator<char> {}
					}
				);
			}
			else std::fprintf(stderr, "Unable to open (%s)", path.c_str());
		}
	}
	for (auto file : State::files) {
		if (State::atlases.contains(file)) continue;
		State::atlases[file] = Atlas {};
	}

	GUI::initiate();

	GUI::BottomRightOverlay();
	GUI::MainMenu();

	if (State::showDemoWindow) ImGui::ShowDemoWindow();

	ImVec4 color = {0.10, 0.10, 0.11, 1.0};
	if (State::dragOver) color.x += 0.3;

	window.render(color);
}



int main(int argc, char const* argv[]) {
	Window window {"Steno Atlas", 1280, 720};

#ifdef __EMSCRIPTEN__
	auto userData = std::tie(window, ImGui::GetIO());
	emscripten_set_main_loop_arg(
		[] (void* data) {
			if (!State::running) emscripten_cancel_main_loop();
			std::apply(mainLoop, *(decltype(userData)*)data);
		},
		&userData, 0, true
	);
#else
	while (State::running) mainLoop(window, ImGui::GetIO());
#endif

}
