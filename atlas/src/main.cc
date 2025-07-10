#include "steno.hh"
#include "window.hh"
#include "canvas.hh"
#include "atlas.hh"
#include "steno_parsers.hh"
#include <istream>
#include <fstream>
#include <iterator>
#include <list>

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
	Texture texture;

	enum Type { Unknown, Text, JSON, RTF };
	Dictionary(std::istream& input, std::string name, Type type)
	: name{name} {
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
		texture = Texture {atlas.getMipmaps(), Atlas::N, Atlas::N};
	}

	Dictionary(std::istream& input, std::filesystem::path path) {
		Type type = Unknown;
		if (std::string extension = path.extension(); !extension.empty()) {
			for (char& c : extension) c = std::tolower(c);
			/**/ if (extension == ".txt" ) type = Text;
			else if (extension == ".json") type = JSON;
			else if (extension == ".rtf" ) type = RTF ;
		}
		*this = Dictionary(input, path.filename(), type);
	}

	void save() const {
		std::filesystem::path imgPath {name};
		imgPath.replace_extension("");
		imgPath += " atlas.png";
		stbi_write_png(
			imgPath.c_str(), Atlas::N, Atlas::N,
			4, atlas.image.data(), 4*Atlas::N
		);
		JS::offerDownload(imgPath.c_str());
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
	// Window state:
	bool running = true;
	// App state:
//	std::optional<float> transferProgress;
	std::list<Dictionary> dictionaries;
	Dictionary const* selectedDictionary = nullptr;
	float atlasScale = 1.0;
	ImVec2 atlasPos = {0.5, 0.5};
	// Web-related state:
	static inline bool dragOver = false;
	static inline bool showDemoWindow = false;
};

extern "C" { // These functions will be called from the browser.
	void setDragOver(bool input) { State::dragOver = input; }
	void showDebug() { State::showDemoWindow = true; }
}

/* ~~ Main Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void mainLoop(Window& window, State& state, Canvas& canvas) {
	// Handle events.
	for (SDL_Event event; SDL_PollEvent(&event);) {
		ImGui_ImplSDL3_ProcessEvent(&event);
		if (event.type == SDL_EVENT_QUIT) state.running = false;
		if (event.type == SDL_EVENT_DROP_FILE) {
			std::filesystem::path path {event.drop.data};
			if (std::ifstream file {path}) {
				Dictionary dict {file, path};
				if (dict.atlas.image.empty()) continue;
				state.dictionaries.push_back(std::move(dict));
			}
			else std::printf("Unable to open %s\n", path.c_str());
		}
		// Keyboard input.
		auto pressed = [&event] (auto ... keys) {
			return event.type == SDL_EVENT_KEY_DOWN
			&&     ((event.key.key == keys) || ... );
		};
		auto const Zoom = std::sqrt(2.0);
		auto const Move = 1.0 / 8.0;
		if (pressed(SDLK_LEFTBRACKET, SDLK_KP_MINUS)) state.atlasScale /= Zoom;
		if (pressed(SDLK_RIGHTBRACKET, SDLK_KP_PLUS)) state.atlasScale *= Zoom;
		if (pressed(SDLK_LEFT )) state.atlasPos.x -= Move / state.atlasScale;
		if (pressed(SDLK_RIGHT)) state.atlasPos.x += Move / state.atlasScale;
		if (pressed(SDLK_UP   )) state.atlasPos.y -= Move / state.atlasScale;
		if (pressed(SDLK_DOWN )) state.atlasPos.y += Move / state.atlasScale;
		state.atlasScale = std::clamp(state.atlasScale, 1.0f/8.0f, 1024.0f);
		state.atlasPos.x = std::clamp(state.atlasPos.x, -2.0f, 2.0f);
		state.atlasPos.y = std::clamp(state.atlasPos.y, -2.0f, 2.0f);
	}
	// Run all GUI code. (Probably should be abstracted into a class.)
#	include "gui.hh"
	canvas.render();
	window.render();
}

int main(int argc, char const* argv[]) {
	State state {};
	Window window {"Steno Atlas Prototype", 1280, 720, &state.running};
	Canvas atlasViewer {
		"assets/atlas.frag",
//		{"Atlas",      Window::Uniform<ImTextureID> {&currentAtlas}};
//		{"Resolution", Window::Uniform<float, 2>    {&fbWidth, fbHeight}};
//		{"Scale",      Window::Uniform<float, 1>    {&state.scale}};
//		{"Position",   Window::Uniform<float, 2>    {&state.position}};
//		{"Mouse",      Window::Uniform<float, 2>    {&state.mouse}};
	};
	atlasViewer.refScale = &state.atlasScale;
	atlasViewer.refPosition = &state.atlasPos;
	window.run(mainLoop, window, state, atlasViewer);
}
