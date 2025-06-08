#include "window.hh"
#include "parsers.hh"
#include "../../steno.hh"
#include <istream>
#include <fstream>
#include <filesystem>
#include <map>
#include <tuple>
#include <iterator>
#include <optional>



struct Dictionary {
#	pragma clang diagnostic ignored "-Wc99-designator"
	using Map = std::map<steno::Stroke, std::string>;
	using ParserFn = std::optional<Map> (std::istream&);
	std::string name;
	Map entries;

	enum Type { Unknown, Text, JSON, RTF };
	Dictionary(std::string name, Type type, std::istream& input): name{name} {
		if (type != Unknown) {
			auto parse = (ParserFn* []) {
				[Text] = &steno::parsePlain,
				[JSON] = &steno::parseJSON ,
				[RTF ] = &steno::parseRTF  ,
			} [type];
			if (!parse) std::printf("Unsupported filetype for %s\n", name.c_str());
			else if (auto result = parse(input)) entries = *result;
		}
		else if (type == Unknown) {
			auto/*    */ result = steno::parseRTF  (input);
			if (!result) result = steno::parseJSON (input);
			if (!result) result = steno::parsePlain(input);
			if (!result) std::printf("Unable to parse %s", name.c_str());
		}
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
	std::vector<Dictionary> dictionaries;
	std::map<Dictionary, Atlas> atlases;
	Atlas* selectedAtlas = nullptr;
}

extern "C" { // These functions will be called from the browser.
	void setDragOver(bool input) { State::dragOver = input; }
}



#include "gui.hh" // dependent on State

void mainLoop(Window& window, ImGuiIO& io) {
	for (SDL_Event event; SDL_PollEvent(&event);) {
		ImGui_ImplSDL3_ProcessEvent(&event);
		if (event.type == SDL_EVENT_QUIT) State::running = false;
		if (event.type == SDL_EVENT_DROP_FILE) {
			std::filesystem::path path {event.drop.data};
			if (std::ifstream file {path}) {
				Dictionary dict {path.filename(), Dictionary::RTF, file};
				if (!dict.entries.empty()) State::dictionaries.push_back(std::move(dict));
			}
			else std::printf("Unable to open %s", path.c_str());
		}
	}
//	for (auto const& file : State::files) {
//		if (State::atlases.contains(file)) continue;
//		State::atlases[file] = Atlas {window};
//	}

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
