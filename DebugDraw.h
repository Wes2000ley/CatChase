#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

#include <glm/glm.hpp>
#include <memory>
#include "SHADER.h"
#include "Collision.h"

void InitDebugDraw();                     // Call once at init
void DestroyDebugDraw();                  // Cleanup on shutdown

void DrawDebugCircle(const Circle& c, const glm::vec3& color, const glm::mat4& proj, std::shared_ptr<Shader> shader, int segments = 32);
void DrawDebugGrid(int cols, int rows, int tileWidth, int tileHeight, const glm::mat4& proj, std::shared_ptr<Shader> shader);

#endif // DEBUG_DRAW_H
