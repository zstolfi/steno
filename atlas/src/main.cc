#include "window.hh"
#include "atlas.hh"
#include "steno.hh"
#include "steno_parsers.hh"
#include <istream>
#include <fstream>
#include <iterator>

/* ~~ App State ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma clang diagnostic push
struct Dictionary {
#	pragma clang diagnostic ignored "-Wc99-designator"
	using ParserFn = steno::ParserDictionaryFn;
	using TextureFn = ImTextureID (std::span<uint8_t const>, int W, int H);
	using TextureFn_Arg = std::function<TextureFn>;
	std::string name;
	steno::Dictionary entries;
	Atlas atlas;
	ImTextureID texture;

	enum Type { Unknown, Text, JSON, RTF };
	Dictionary(
		std::istream& input,
		std::string name, Type type,
		TextureFn_Arg loadTexture
	): name{name} {
		std::istreambuf_iterator<char> begin {input}, end {};
		std::vector<char> bytes {begin, end};
		auto tryParser = std::bind_front(&Dictionary::tryParser, this, bytes);
		auto parse = (ParserFn* []) {
			[Text] = steno::parsePlain,
			[JSON] = steno::parseJSON,
			[RTF]  = steno::parseRTF,
		} [type];
		/**/ if (!parse && tryParser(steno::parseGuess));
		else if (!parse) { std::printf("Unkown filetype for %s\n", name.c_str()); return; }
		else if (tryParser(parse) || tryParser(steno::parseGuess));
		else { std::printf("Parse failed for %s\n", name.c_str()); return; }
		atlas = Atlas {entries};
		texture = loadTexture(atlas.image, Atlas::N, Atlas::N);
	}

	Dictionary(
		std::istream& input,
		std::filesystem::path path,
		TextureFn_Arg loadTexture
	) {
		Type type = Unknown;
		if (std::string extension = path.extension(); !extension.empty()) {
			for (char& c : extension) c = std::tolower(c);
			/**/ if (extension == ".txt" ) type = Text;
			else if (extension == ".json") type = JSON;
			else if (extension == ".rtf" ) type = RTF ;
		}
		*this = Dictionary(input, path.filename(), type, loadTexture);
	}

private:
	bool tryParser(std::vector<char> const& bytes, ParserFn* parse) {
		auto result = parse(bytes);
		if (result) this->entries = *result;
		return (bool)result;
	}
};
#pragma clang diagnostic pop

struct State {
	bool running = true;
	// App state:
	bool showDemoWindow = false;
	ImVec4 backgroundColor = {0, 0, 0, 0};
//	std::optional<float> transferProgress;
	std::vector<Dictionary> dictionaries;
	// Web-related state:
	static inline bool dragOver = false;
};

extern "C" { // These functions will be called from the browser.
	void setDragOver(bool input) { State::dragOver = input; }
}

/* ~~ Main Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void mainLoop(Window& window, State& state) {
	auto loadTexture = std::bind_front(&Window::loadTexture, &window);
	// Handle events.
	for (SDL_Event event; SDL_PollEvent(&event);) {
		ImGui_ImplSDL3_ProcessEvent(&event);
		if (event.type == SDL_EVENT_QUIT) state.running = false;
		if (event.type == SDL_EVENT_DROP_FILE) {
			std::filesystem::path path {event.drop.data};
			if (std::ifstream file {path}) {
				Dictionary dict {file, path, loadTexture};
				if (dict.atlas.image.empty()) continue;
				state.dictionaries.push_back(std::move(dict));
			}
			else std::printf("Unable to open %s\n", path.c_str());
		}
	}
	// Run all GUI code. (Probably should be abstracted into a class.)
#	include "gui.hh"
	window.render(state.backgroundColor);
}

int main(int argc, char const* argv[]) {
	State state {};
	Window window {"Steno Atlas Prototype", 1280, 720, &state.running};
	window.run(mainLoop, window, state);
}
