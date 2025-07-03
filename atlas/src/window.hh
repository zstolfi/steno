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

GLuint VAO, VBO, FBO, RBO
,      textureID, shaderID
,      textureEmpty, currentAtlas
,      fbWidth = 800, fbHeight = 600;

GLint u_Atlas, u_Resolution, u_Scale, u_Position;

char const* vertexShaderSource = R"`(#version 300 es
precision mediump float;
layout (location = 0) in vec2 Position;

void main() {
	gl_Position = vec4(Position, 0.0, 1.0);
}
)`";

char const* fragmentShaderSource = R"`(#version 300 es
precision mediump float;
// Uniform variables (in order of importance):
uniform sampler2D Atlas;
uniform vec2 Resolution;
uniform float Scale;
uniform vec2 Position;
//uniform vec2 Mouse;
//uniform bvec2 MouseClick;
//uniform float Time;

layout (location = 0) out vec4 FragColor;

vec3 getAtlasColor(vec2 uv) {
	return texture(Atlas, uv).xyz;
}

void main() {
	// View the entire Atlas centering the canvas at Scale = 1.0.
	float zoom = max(1.0/Resolution.x, 1.0/Resolution.y);
	vec2 uv = (gl_FragCoord.xy - 0.5*Resolution) * zoom/Scale + Position;
	// Color everything else black.
	bool onAtlas = max(abs(uv.x - 0.5), abs(uv.y - 0.5)) <= 0.5;
	vec3 color = onAtlas? getAtlasColor(uv): vec3(0.0);
	// Output.
	FragColor = vec4(color, 1.0);
}
)`";

void createEmptyTexture() {
	auto constexpr N = 256;
	std::vector<uint8_t> t (4*N*N, 0);
	for (int y=0; y<N; y++)
	for (int x=0; x<N; x++) {
		std::size_t i = y*N + x;
		t[4*i+0] = x^y;
		t[4*i+1] = x^y;
		t[4*i+2] = x^y;
		t[4*i+3] = 255;
	}

	glGenTextures(1, &textureEmpty);
	glBindTexture(GL_TEXTURE_2D, textureEmpty);
	currentAtlas = textureEmpty;

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Upload pixels into texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N, N, 0, GL_RGBA, GL_UNSIGNED_BYTE, t.data());
}

void createCanvas() {
	GLfloat vertices[] = {
		-1.0,	+1.0 ,  // Top-Left
		+1.0,	+1.0 ,  // Top-Rright
		-1.0,	-1.0 ,  // Bottom-Left
		+1.0,	-1.0 ,  // Bottom-Rright
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void addShader(GLuint program, std::string source, GLenum type) {
	GLuint shader = glCreateShader(type);
	auto ptr = source.c_str();
	glShaderSource(shader, 1, &ptr, nullptr);
	glCompileShader(shader);

	GLint success {}, logLength {};
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	if (!success) {
		std::string log (logLength-1, '\0');
		glGetShaderInfoLog(shader, logLength, nullptr, log.data());
		std::printf("Error compiling shader:\n%s\n", log.c_str());
	}
	glAttachShader(program, shader);
}

void createShaders() {
	if (shaderID = glCreateProgram(), !shaderID) {
		std::printf("Error creating shader program.\n");
		return;
	}

	addShader(shaderID, vertexShaderSource, GL_VERTEX_SHADER);
	addShader(shaderID, fragmentShaderSource, GL_FRAGMENT_SHADER);

	glLinkProgram(shaderID);
	{
		GLint success {}, logLength {};
		glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
		glGetProgramiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
		if (!success) {
			std::string log (logLength-1, '\0');
			glGetProgramInfoLog(shaderID, logLength, nullptr, log.data());
			std::printf("Error linking program:\n%s\n", log.c_str());
			return;
		}
	}

	glValidateProgram(shaderID);
	{
		GLint success {}, logLength {};
		glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &success);
		glGetProgramiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
		if (!success) {
			std::string log (logLength-1, '\0');
			glGetProgramInfoLog(shaderID, logLength, nullptr, log.data());
			std::printf("Error validating program:\n%s\n", log.c_str());
			return;
		}
	}

	// Set uniforms.
	glUseProgram(shaderID);
	u_Atlas      = glGetUniformLocation(shaderID, "Atlas"     );
	u_Resolution = glGetUniformLocation(shaderID, "Resolution");
	u_Scale      = glGetUniformLocation(shaderID, "Scale"     );
	u_Position   = glGetUniformLocation(shaderID, "Position"  );

	createEmptyTexture();
	glUniform1i(u_Atlas, 0/*GL_TEXTURE0*/);
}

void rescaleFramebuffer(GLsizei width, GLsizei height) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
}

void createFramebuffer() {
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &textureID);
	glGenRenderbuffers(1, &RBO);
	rescaleFramebuffer(800, 600);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::printf("Error completing framebuffer.\n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void bindFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void unbindFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

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

		// WebGL-friendly subset of OpenGL ES 3.0
		char const* glslVersion = "#version 300 es";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

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
		ImGui_ImplOpenGL3_Init(glslVersion);

		createCanvas();
		createShaders();
		createFramebuffer();

		// Cutsom JS interfaces
		enableDragAndDrop();
	}

	void render(ImVec4 clearColor = {0, 0, 0, 1}) {
		ImGui::Render();
		
		bindFramebuffer();
		rescaleFramebuffer(fbWidth, fbHeight);

		glUseProgram(shaderID);
		glUniform2f(u_Resolution, fbWidth, fbHeight);
		glUniform1f(u_Scale, 1.0);
		glUniform2f(u_Position, 0.5, 0.5);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, currentAtlas);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glBindVertexArray(0);
		glUseProgram(0);
		unbindFramebuffer();

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
		glDeleteTextures(textures.size(), textures.data());

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

		SDL_GL_DestroyContext(glContext);
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
