#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <string>
#include <vector>
#include <span>
#include <tuple>
#include <functional>
#include <variant>
#include <filesystem>
#include <cstdio>
#include <cstdint>
#include <cctype>

#ifdef __EMSCRIPTEN__
#	include <emscripten.h>
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

extern "C" { void jsDownload(char const*); }

class Window {
	SDL_Window* winPtr;
	SDL_GLContext gl_context;
	std::vector<GLuint> textures;

	using RunBool = bool const*;
	using RunFunc = bool (*)();
	std::variant<RunBool, RunFunc> runPred;

public:
	Window(std::string title, unsigned W, unsigned H, decltype(runPred) rp) {
		// Know when the user code stops running
		runPred = rp;
		// Setup SDL
		if (!SDL_Init(SDL_INIT_VIDEO)) {
			std::printf("Error: SDL_Init(): %s\n", SDL_GetError());
			exit(-1);
		}

		// Decide GL+GLSL versions
#	if defined(IMGUI_IMPL_OPENGL_ES2)
		// GL ES 2.0 + GLSL 100 (WebGL 1.0)
		const char* glsl_version = "#version 100";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#	elif defined(IMGUI_IMPL_OPENGL_ES3)
		// GL ES 3.0 + GLSL 300 es (WebGL 2.0)
		const char* glsl_version = "#version 300 es";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#	elif defined(__APPLE__)
		// GL 3.2 Core + GLSL 150
		const char* glsl_version = "#version 150";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#	else
		// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#	endif

		// Create window with graphics context
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		auto window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
		this->winPtr = SDL_CreateWindow(title.c_str(), W, H, window_flags);
		if (!winPtr) {
			std::printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
			exit(-1);
		}
		this->gl_context = SDL_GL_CreateContext(winPtr);
		if (!gl_context) {
			std::printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
			exit(-1);
		}

		SDL_GL_MakeCurrent(winPtr, gl_context);
		SDL_GL_SetSwapInterval(1); // Enable vsync
		SDL_SetWindowPosition(winPtr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		SDL_ShowWindow(winPtr);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

		// Setup Platform/Renderer backends
		ImGui_ImplSDL3_InitForOpenGL(winPtr, gl_context);
		ImGui_ImplOpenGL3_Init(glsl_version);

		// Cutsom JS interfaces
		enableDragAndDrop();
	}

	void render(ImVec4 clearColor = {0, 0, 0, 1}) {
		ImGui::Render();
		ImGuiIO& io = ImGui::GetIO();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(winPtr);
	}

	void run(auto&& mainLoop) {
#	ifdef __EMSCRIPTEN__
		static std::function<void ()> sMainLoop = mainLoop;
		emscripten_set_main_loop([] { sMainLoop(); }, 0, true);
#	else
		while (running()) mainLoop();
#	endif
	}

	template <template<class> class Tuple, class ... Args>
	void run(auto&& mainLoop, Tuple<Args ... >&& userData) {
#	ifdef __EMSCRIPTEN__
		static std::function<void (Args ... )> sMainLoop = mainLoop;
		static Tuple<Args ... > sUserData = userData;
		emscripten_set_main_loop_arg(
			[] (void* data) {
				std::apply(sMainLoop, *(Tuple<Args ... >*)data);
			},
			(void*)&sUserData, 0, true
		);
#	else
		while (running()) std::apply(mainLoop, userData);
#	endif
	}

	~Window() {
		glDeleteTextures(textures.size(), textures.data());

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

		SDL_GL_DestroyContext(gl_context);
		SDL_DestroyWindow(winPtr);
		SDL_Quit();
	}

	[[nodiscard]]
	ImTextureID loadTexture(std::span<uint8_t const> data, int W, int H) {
		IM_ASSERT(data.size() == W*H*4);

		// Create a OpenGL texture identifier
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Upload pixels into texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		this->textures.push_back(texture);
		return (ImTextureID)(intptr_t)texture;
	}

	void download(std::filesystem::path path) {
		jsDownload(path.c_str());
	}

private:
	bool running() const {
		if (RunBool const* val = std::get_if<RunBool>(&runPred)) return **val;
		if (RunFunc const* val = std::get_if<RunFunc>(&runPred)) return (*val)();
		return false;
	}

	void enableDragAndDrop() const {
#	ifdef __EMSCRIPTEN__
		EM_ASM(
			const setDragOver = Module.cwrap("setDragOver", "", ["boolean"]);
			const Set = (state) => (event) => {
				event.preventDefault();
				setDragOver(state);
			};
			Module.canvas.addEventListener("dragenter", Set(true));
			Module.canvas.addEventListener("dragover" , Set(true));
			Module.canvas.addEventListener("dragleave", Set(false));
			Module.canvas.addEventListener("drop"     , Set(false));
		);
#	endif
	}
};

#ifdef __EMSCRIPTEN__
EM_JS(void, jsDownload, (char const* path_raw), {
	const path = UTF8ToString(path_raw);
	const content = Module.FS.readFile(path);
	console.log(`Downloading ${path} (${content.length} bytes)`);
	// None shall bypass the <a>.
	let a = document.createElement("a");
	a.download = path;
	a.href = URL.createObjectURL(new Blob([content]));
	a.style.display = "none";
	document.body.appendChild(a);
	a.click();
	setTimeout(() => {
		document.body.removeChild(a);
		URL.revokeObjectURL(a.href);
	}, 1000);
});
#endif
