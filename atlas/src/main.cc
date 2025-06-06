#include "window.hh"
#include <fstream>
#include <vector>
#include <map>
#include <tuple>
#include <iterator>



struct File {
	std::string name;
	std::span<uint8_t const> bytes;
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
	void receiveFile(char const* name, std::size_t size, uint8_t const* bytes) {
		std::printf("File: (%s) received in %zu bytes\n", name, size);
		State::files.emplace_back(name, std::span {bytes, size});
	}
}



void mainLoop(Window& window, ImGuiIO& io) {
	for (SDL_Event e; SDL_PollEvent(&e);) {
		ImGui_ImplSDL3_ProcessEvent(&e);
		if (e.type == SDL_EVENT_QUIT) State::running = false;
	}
	for (auto file : State::files) {
		if (State::atlases.contains(file)) continue;
		State::atlases[file] = Atlas {};
	}

	window.newFrame();
	
	/*Main Window*/
	ImGui::SetNextWindowSize(ImVec2 {500, 400}, ImGuiCond_FirstUseEver);
	ImGui::Begin("Steno Atlas Pre-Prototype");
	if (State::files.empty()) {
		ImGui::Text("Drag & drop files here to get started!");
	}
	else {
		ImGui::Text("Number of files loaded: %zu", State::files.size());
		for (int i=0; auto file : State::files) {
			ImGui::PushID(i++);
			if (ImGui::TreeNode(file.name.c_str())) {
				ImGui::Text("%zu bytes", file.bytes.size());
				ImGui::SameLine();
				if (ImGui::Button("Print")) file.print();
				if (auto it = State::atlases.find(file); it != State::atlases.end()) {
					auto const& atlas = it->second;
					ImGui::Text("Atlas (default image for now)");
					ImGui::Indent();
					ImGui::Image((ImTextureID)(intptr_t)atlas.texture, ImVec2 {256, 256});
					ImGui::Unindent();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
	ImGui::End();

	/*Bottom-Right Overlay*/ {
		auto const* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
		ImVec2 work_size = viewport->WorkSize;
		float const PAD = 10.0f;
		ImVec2 windowPos;
		windowPos.x = work_pos.x + work_size.x - PAD;
		windowPos.y = work_pos.y + work_size.y - PAD;

		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2 {1, 1});
		ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
		ImGui::Begin("Programmer Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
		ImGui::Checkbox("I'm Bored", &State::showDemoWindow);
		ImGui::End();
	}

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
