#include "window.hh"
#include <fstream>
#include <filesystem>
#include <map>
#include <tuple>
#include <iterator>



struct File {
	std::string name;
	std::vector<uint8_t> bytes;
	auto operator<=>(File const& f) const { return name <=> f.name; }

	File(std::filesystem::path path) {
		name = path.filename();
		if (std::ifstream input {path}) {
			bytes.reserve(std::filesystem::file_size(path));
			std::istreambuf_iterator<char> const begin {input}, end {};
			for (auto it=begin; it!=end; ++it) bytes.push_back(*it);
			std::printf("File: (%s) received in %zu bytes\n"
			,	name.c_str(), bytes.size());
		}
		else std::fprintf(stderr, "Unable to open (%s)", path.c_str());
	}

	void print() const {
		for (char c : bytes) std::putchar(c);
		if (bytes.back() != '\n') std::putchar('\n');
	}
};

struct Atlas {
	std::vector<uint8_t> image;
	ImTextureID texture;
	Atlas() = default;
	Atlas(Window& window) : image(256*256*4) {
		for (unsigned y=0; y<256; y++)
		for (unsigned x=0; x<256; x++) {
			std::size_t i = 256*y + x;
			image[4*i + 0] = x^y;
			image[4*i + 1] = x^y;
			image[4*i + 2] = x^y;
			image[4*i + 3] = 255;
		}
		texture = window.loadTexture(image, 256, 256);
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
	for (SDL_Event event; SDL_PollEvent(&event);) {
		ImGui_ImplSDL3_ProcessEvent(&event);
		if (event.type == SDL_EVENT_QUIT) State::running = false;
		if (event.type == SDL_EVENT_DROP_FILE) {
			State::files.push_back(File {event.drop.data});
		}
	}
	for (auto const& file : State::files) {
		if (State::atlases.contains(file)) continue;
		State::atlases[file] = Atlas {window};
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
