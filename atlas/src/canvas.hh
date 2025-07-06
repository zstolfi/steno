#pragma once
#include "window.hh"
#include <fstream>

class Canvas {
	GLuint VAO, VBO, FBO, RBO;
	GLuint program;
	GLuint texture, currentAtlas {};
	unsigned width = 800, height = 600;
	// Uniforms:
	GLint u_Atlas, u_Resolution, u_Scale, u_Position;

public:
	Canvas(std::filesystem::path path) {
		if (std::ifstream file {path}) {
			std::istreambuf_iterator<char> begin {file}, end {};
			std::string fragmentSource {begin, end};

			createTriangles();
			createShader(fragmentSource);
			createFramebuffer();
		}
		else std::printf("Error opening shader %s\n", path.c_str());
	}

	void rescale(int w, int h) {
		width = w, height = h;
	}

	void setAtlas(ImTextureID newAtlas) {
		currentAtlas = newAtlas;
	}

	void render() {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		applyFramebufferSize();

		glUseProgram(program);
		glUniform2f(u_Resolution, width, height);
		glUniform1f(u_Scale, 1.0);
		glUniform2f(u_Position, 0.5, 0.5);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, currentAtlas);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindVertexArray(0);
		glUseProgram(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	[[nodiscard]]
	ImTextureID getTexture() const {
		return (ImTextureID)texture;
	}

private:
	void createTriangles() {
		constexpr GLfloat vertices[] {
			-1.0,	+1.0 ,  // Top-Left
			+1.0,	+1.0 ,  // Top-Rright
			-1.0,	-1.0 ,  // Bottom-Left
			+1.0,	-1.0 ,  // Bottom-Right
		};

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void compileShader(std::string source, GLenum type) {
		// Array of source strings to be concatenated.
		std::vector ptrs {GLSLVersion, GLSLPrecision, source.c_str()};
		// Create and bind vertex/fragment shader.
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, ptrs.size(), ptrs.data(), nullptr);
		glCompileShader(shader);
		checkShaderStatus(shader, GL_COMPILE_STATUS, "Error compiling shader");
		glAttachShader(program, shader);
	}

	void createShader(std::string fragmentSource) {
		// Create program.
		program = glCreateProgram();
		if (!program) std::printf("Error creating shader program.\n");
		// Bare-bones vertex shader, for our two triangles.
		constexpr auto VertexSource = R"`(
			layout (location = 0) in vec2 Position;
			void main() {
				gl_Position = vec4(Position, 0.0, 1.0);
			}
		)`";
		// Compile.
		compileShader(VertexSource, GL_VERTEX_SHADER);
		compileShader(fragmentSource, GL_FRAGMENT_SHADER);
		// Link.
		glLinkProgram(program);
		checkProgramStatus(program, GL_LINK_STATUS, "Error linking program");
		// Validate.
		glValidateProgram(program);
		checkProgramStatus(program, GL_VALIDATE_STATUS, "Error validating program");
		// Set uniforms.
		glUseProgram(program);
		u_Atlas      = glGetUniformLocation(program, "Atlas"     );
		u_Resolution = glGetUniformLocation(program, "Resolution");
		u_Scale      = glGetUniformLocation(program, "Scale"     );
		u_Position   = glGetUniformLocation(program, "Position"  );
		// The texture uniform is special.
		glUniform1i(u_Atlas, 0/*GL_TEXTURE0*/);
	}

	void applyFramebufferSize() {
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	}

	void createFramebuffer() {
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glGenTextures(1, &texture);
		glGenRenderbuffers(1, &RBO);
		applyFramebufferSize();

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::printf("Error completing framebuffer.\n");
		}
		// Unbind.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	bool checkStatus_impl(
		decltype(glGetProgramiv) get,
		decltype(glGetProgramInfoLog) logGet,
		GLuint program,
		GLenum query,
		char const* message
	) {
		GLint success {}, logLength {};
		get(program, query, &success);
		get(program, GL_INFO_LOG_LENGTH, &logLength);
		if (!success) {
			std::string log (logLength-1, '\0');
			logGet(program, logLength, nullptr, log.data());
			std::printf("%s\n%s\n", message, log.c_str());
		}
		return success;
	}

	bool checkProgramStatus(GLuint p, GLenum q, char const* m)
	{ return checkStatus_impl(glGetProgramiv, glGetProgramInfoLog, p, q, m); }

	bool checkShaderStatus(GLuint p, GLenum q, char const* m)
	{ return checkStatus_impl(glGetShaderiv, glGetShaderInfoLog, p, q, m); }
};
