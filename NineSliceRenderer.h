#pragma once
#include <memory>
#include <GL/gl.h>

#include "texture.h"
#include "shader.h"
#include <glm/glm.hpp>

class NineSliceRenderer {
public:
	explicit NineSliceRenderer(std::shared_ptr<Shader> shader);
	~NineSliceRenderer();

	void SetTexture(std::shared_ptr<Texture2D> texture, int border);
	void Render(float x, float y, float width, float height, const glm::mat4& projection);

private:
	GLuint VAO = 0, VBO = 0;
	std::shared_ptr<Shader> shader_;
	std::shared_ptr<Texture2D> texture_;
	int border_ = 16;

	void initRenderData();
};
