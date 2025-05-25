#include "NineSliceRenderer.h"
#include <glad/glad.h>
#include <array>

NineSliceRenderer::NineSliceRenderer(std::shared_ptr<Shader> shader)
	: shader_(std::move(shader)) {
	initRenderData();
}

NineSliceRenderer::~NineSliceRenderer() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void NineSliceRenderer::SetTexture(std::shared_ptr<Texture2D> texture, int border) {
	texture_ = std::move(texture);
	border_ = border;
}

void NineSliceRenderer::initRenderData() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0); // pos + uv
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void NineSliceRenderer::Render(float x, float y, float w, float h, const glm::mat4 &projection) {
	if (!texture_ || !shader_) return;

	float tw = static_cast<float>(texture_->Width);
	float th = static_cast<float>(texture_->Height);
	float b = static_cast<float>(border_);

	struct Quad {
		float px, py, pw, ph; // position
		float tx, ty, tw, th; // texcoords
	};

	std::array<Quad, 9> slices = {
		{
			// Top row
			{x, y, b, b, 0.0f, 0.0f, b, b}, // top-left
			{x + b, y, w - 2 * b, b, b, 0.0f, tw - 2 * b, b}, // top
			{x + w - b, y, b, b, tw - b, 0.0f, b, b}, // top-right

			// Middle row
			{x, y + b, b, h - 2 * b, 0.0f, b, b, th - 2 * b}, // left
			{x + b, y + b, w - 2 * b, h - 2 * b, b, b, tw - 2 * b, th - 2 * b}, // center
			{x + w - b, y + b, b, h - 2 * b, tw - b, b, b, th - 2 * b}, // right

			// Bottom row
			{x, y + h - b, b, b, 0.0f, th - b, b, b}, // bottom-left
			{x + b, y + h - b, w - 2 * b, b, b, th - b, tw - 2 * b, b}, // bottom
			{x + w - b, y + h - b, b, b, tw - b, th - b, b, b} // bottom-right
		}
	};

	shader_->Use();
	shader_->SetMatrix4("projection", projection);
	shader_->SetInteger("image", 0);

	glActiveTexture(GL_TEXTURE0);
	texture_->Bind();
	glBindVertexArray(VAO);

	for (const auto &quad: slices) {
		float vertices[6][4] = {
			{quad.px, quad.py + quad.ph, quad.tx / tw, (quad.ty + quad.th) / th},
			{quad.px + quad.pw, quad.py, (quad.tx + quad.tw) / tw, quad.ty / th},
			{quad.px, quad.py, quad.tx / tw, quad.ty / th},

			{quad.px, quad.py + quad.ph, quad.tx / tw, (quad.ty + quad.th) / th},
			{quad.px + quad.pw, quad.py + quad.ph, (quad.tx + quad.tw) / tw, (quad.ty + quad.th) / th},
			{quad.px + quad.pw, quad.py, (quad.tx + quad.tw) / tw, quad.ty / th},
		};

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
