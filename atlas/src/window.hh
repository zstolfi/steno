#pragma once
#include <SDL3/SDL.h>
#include <GL/glew.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <string>
#include <vector>
#include <span>
#include <tuple>
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

// WebGL-friendly subset of OpenGL ES 3.0 (https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html)
constexpr auto GLVersion = (int []) {3, 0};
constexpr auto GLVersionProfile = SDL_GL_CONTEXT_PROFILE_ES;
constexpr auto GLSLVersion = "#version 300 es\n";
constexpr auto GLSLPrecision = "precision mediump float;\n";

/* ~~ Window Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

class Window {
	SDL_Window* winPtr;
	SDL_GLContext glContext;
	std::vector<GLuint> textures;

	struct RunPredicate {
		std::variant<bool const*, bool (*)()> userPred;
		RunPredicate(auto p) : userPred{p} {}
		bool operator()();
	} running;

public:
	Window(std::string title, unsigned W, unsigned H, RunPredicate rp)
	: running{rp} {
		// Setup SDL
		if (!SDL_Init(SDL_INIT_VIDEO)) {
			std::printf("Error: SDL_Init(): %s\n", SDL_GetError());
			exit(-1);
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, GLVersionProfile);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GLVersion[0]);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GLVersion[1]);

		// Create window with graphics context
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		auto const windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
		this->winPtr = SDL_CreateWindow(title.c_str(), W, H, windowFlags);
		if (!winPtr) {
			std::printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
			exit(-1);
		}
		this->glContext = SDL_GL_CreateContext(winPtr);
//		::glewExperimental = GL_TRUE;
		if (auto status = glewInit(); status != GLEW_OK) {
			std::printf("Error: glewInit(): %s\n", glewGetErrorString(status));
			exit(-1);
		}

		SDL_GL_MakeCurrent(winPtr, glContext);
		SDL_GL_SetSwapInterval(1); // Enable vsync
		SDL_SetWindowPosition(winPtr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		SDL_ShowWindow(winPtr);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

		// Setup Platform/Renderer backends
		ImGui_ImplSDL3_InitForOpenGL(winPtr, glContext);
		ImGui_ImplOpenGL3_Init(GLSLVersion);

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

	template <class ... Args>
	void run(auto&& mainLoop, Args&& ... userArgs) {
#	ifdef __EMSCRIPTEN__
		using Tuple = std::tuple<Args&& ... >;
		// 'run' is only called once, so we can create static copies of our
		// variables for our captureless lambda to use.
		static auto  sRunning  { this->running };
		static auto  sMainLoop { std::move(mainLoop) };
		static Tuple sUserArgs { std::forward<Args>(userArgs) ... };
		emscripten_set_main_loop_arg(
			[] (void* args) {
				if (!sRunning()) emscripten_cancel_main_loop();
				else std::apply(sMainLoop, *(Tuple*)args);
			},
			(void*)&sUserArgs, 0, true
		);
#	else
		while (running()) mainLoop(std::forward<Args>(userArgs) ... );
		std::exit(0);
#	endif
	}

	~Window() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

		SDL_GL_DestroyContext(glContext);
		SDL_DestroyWindow(winPtr);
		SDL_Quit();
	}

private:
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

bool Window::RunPredicate::operator()() {
	using RunBool = bool const*;
	using RunFunc = bool (*)();
	if (RunBool const* val = std::get_if<RunBool>(&userPred)) return **val;
	if (RunFunc const* val = std::get_if<RunFunc>(&userPred)) return (*val)();
	return false;
}

/* ~~ Texture Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

class Texture {
	GLuint ID {};
	using ImageData = std::span<uint8_t const>;

public:
	Texture() = default;

	Texture(ImageData pixels, int W, int H) {
		// Create OpenGL texture identifier.
		glGenTextures(1, &this->ID);
		glBindTexture(GL_TEXTURE_2D, this->ID);
		// Setup filtering parameters for display.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// Upload pixels into texture.
		loadTexture(pixels, W, H);
	}

	ImTextureID get() const {
		return (ImTextureID)this->ID;
	}

private:
	void loadTexture(ImageData pixels, int W, int H, int level = 0) {
		IM_ASSERT(W*H != 0);
		IM_ASSERT(pixels.size() == 4*W*H);
		glTexImage2D(
			GL_TEXTURE_2D, level, GL_RGBA, W, H, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, pixels.data()
		);
	}
};

/* ~~ JavaScript Utilities ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef __EMSCRIPTEN__
namespace JS {

EM_JS(void, offerDownload, (char const* path_raw), {
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

} // namespace JS
#endif
