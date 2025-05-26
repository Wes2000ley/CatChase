#pragma once
// ──────────────────────────────────────────────────────────────────────────────
// NineSliceRenderer.h – draws a scalable “nine-slice” panel
// ──────────────────────────────────────────────────────────────────────────────
#include <memory>
#include <glm/glm.hpp>
#include "../../SHADER.h"
#include "../../TEXTURE.h"
#include <glad/glad.h>

class NineSliceRenderer {
public:
	explicit NineSliceRenderer(std::shared_ptr<Shader> shader);
	~NineSliceRenderer();

	/** Assign the texture to slice and the border thickness in *pixels*. */
	void SetTexture(std::shared_ptr<Texture2D> texture, int border_pixels);

	/**
	 * Draw a panel at (x,y) with size (w,h) in *screen / world* units expected
	 * by the caller’s orthographic projection.
	 *
	 * `projection` must be the same matrix you pass to everything else.
	 * `tint` multiplies the fragment colour (default white = unchanged).
	 */
	void Render(float x, float y, float w, float h,
				const glm::mat4& proj,
				const glm::vec4& tint = glm::vec4(1.0f));

	// add after Render() declaration
	static void SetDepthTest(bool enable);


private:
	void initRenderData();

	std::shared_ptr<Shader>   shader_;
	std::shared_ptr<Texture2D> texture_;
	unsigned int VAO{}, VBO{};
	int border_{0};
};
