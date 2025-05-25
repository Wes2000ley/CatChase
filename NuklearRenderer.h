#pragma once

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_BOOL
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_DEFAULT_FONT

#include <nuklear.h>


#include "shader.h"
#include "texture.h"

class NuklearRenderer {
public:
	explicit NuklearRenderer(GLFWwindow* window);
	~NuklearRenderer();

	void BeginFrame();
	void EndFrame();
	struct nk_context* GetContext();

private:
	void InitResources();
	void UploadFont();

	struct nk_context ctx_;
	struct nk_buffer cmds_;
	struct nk_font_atlas atlas_;
	struct nk_draw_null_texture nullTex_;

	GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;

	std::shared_ptr<Shader> shader_;
	std::shared_ptr<Texture2D> fontTexture_;

	GLFWwindow* window_;
	int width_, height_;
	float fbScaleX_, fbScaleY_;
};
