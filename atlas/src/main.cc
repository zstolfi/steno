#include "steno.hh"
#include "window.hh"
#include "canvas.hh"
#include "atlas.hh"
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
	// Window state
	bool running = true;
	// Web-controlled state
	static inline bool dragOver = false;
	static inline bool showDemoWindow = false;
//	std::optional<float> transferProgress;
	// Atlas state
	float aScale = 1.0;
	ImVec2 aPosition = {0.5, 0.5};
	// Dictionary state
	std::vector<Dictionary> dictionaries;

public: // Member functions
	// Atlas transformers
	void aZoom(float factor) {
		aScale *= ::pow(2.0, factor);
	}

	void aMove(ImVec2 direction) {
		aPosition.x += direction.x / aScale;
		aPosition.y += direction.y / aScale;
	}

	// Dictionary getters
	Dictionary const* selectedDictionary() const {
		if (selectedDictionaryIndex == NoDictionaryIndex) return nullptr;
		else return &dictionaries[selectedDictionaryIndex];
	}

	// Dictionary setters
	void selectDictionary(int i) {
		selectedDictionaryIndex = i;
	}

	void openDict(std::filesystem::path path) {
		if (std::ifstream file {path}) {
			Dictionary dict {file, path};
			if (dict.atlas.image.empty()) return;
			dictionaries.push_back(std::move(dict));
			selectedDictionaryIndex = dictionaries.size()-1;
		}
		else std::printf("Unable to open %s\n", path.c_str());
	}

	void downloadDefaultDictionary() {
		std::unique_ptr<char const> path {JS::downloadDefaultDictionary()};
		if (path) openDict(path.get());
	}

private: // Details
	static constexpr int NoDictionaryIndex = -1;
	int selectedDictionaryIndex = NoDictionaryIndex;
};

extern "C" { // These functions will be called from the browser.
	void setDragOver(bool input) { State::dragOver = input; }
	void showDebug() { State::showDemoWindow = true; }
}

/* ~~ Main Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void mainLoop(Window& window, State& state, Canvas& canvas) {
	auto atlasCoordinates = [&] (ImVec2 resolution, ImVec2 mouse)
	-> std::optional<std::array<unsigned, 2>> {
		if (!(0 <= mouse.x && mouse.x < resolution.x)) return {};
		if (!(0 <= mouse.y && mouse.y < resolution.y)) return {};
		float zoom = canvas.zoom / state.aScale;
		ImVec2 pos {
			(mouse.x - 0.5f*resolution.x) * zoom + state.aPosition.x,
			(mouse.y - 0.5f*resolution.y) * zoom + state.aPosition.y,
		};
		if (!(0 <= pos.x && pos.x < 1) || !(0 <= pos.y && pos.y < 1)) return {};
		return std::array {unsigned(2048*pos.x), unsigned(2048*(1-pos.y))};
	};

	// Run all GUI code. (Probably should be abstracted into a class.)
#	include "gui.hh"

	// Handle events.
	std::map<SDL_Keycode, bool> pressed {};
	for (SDL_Event event; SDL_PollEvent(&event);) {
		ImGui_ImplSDL3_ProcessEvent(&event);
		if (event.type == SDL_EVENT_QUIT) state.running = false;
		if (event.type == SDL_EVENT_DROP_FILE) state.openDict(event.drop.data);
		if (event.type == SDL_EVENT_KEY_DOWN) pressed[event.key.key] = true;
	}
	ImGuiIO const& io = ImGui::GetIO();
	// Zooming
	// TODO: Zoom with the cursor as the fixed point.
	if (io.MouseWheel) state.aZoom(0.25 * io.MouseWheel);
	if (pressed[SDLK_LEFTBRACKET ]) state.aZoom(-0.5);
	if (pressed[SDLK_RIGHTBRACKET]) state.aZoom(+0.5);
	if (pressed[SDLK_KP_MINUS    ]) state.aZoom(-0.5);
	if (pressed[SDLK_KP_PLUS     ]) state.aZoom(+0.5);
	// Panning
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		state.aMove(ImVec2 {canvas.zoom * -io.MouseDelta.x
		,                   canvas.zoom * -io.MouseDelta.y});
	}
	if (pressed[SDLK_LEFT ]) state.aMove(ImVec2 {-0.125, 0});
	if (pressed[SDLK_RIGHT]) state.aMove(ImVec2 {+0.125, 0});
	if (pressed[SDLK_UP   ]) state.aMove(ImVec2 {0, -0.125});
	if (pressed[SDLK_DOWN ]) state.aMove(ImVec2 {0, +0.125});
	// Boundaries
	state.aScale = std::clamp(state.aScale, 1.0f/8.0f, 1024.0f);
	state.aPosition.x = std::clamp(state.aPosition.x, -2.0f, 2.0f);
	state.aPosition.y = std::clamp(state.aPosition.y, -2.0f, 2.0f);
	// Render
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
	atlasViewer.refScale = &state.aScale;
	atlasViewer.refPosition = &state.aPosition;
	window.run(mainLoop, window, state, atlasViewer);
}
