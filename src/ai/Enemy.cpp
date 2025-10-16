#include "Enemy.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <utility>
#include "../ai-player/Collision.h"

#include "../game/TileMap.h"

unsigned int Enemy::quadVAO_ = 0;
unsigned int Enemy::quadVBO_ = 0;

Enemy::Enemy(std::shared_ptr<Shader> shader,
             std::shared_ptr<Texture2D> texture,
             glm::vec2 position,
             glm::ivec2 frame,
             float sheetWidth,
             float sheetHeight,
             int frameCols,
             int frameRows)
    : shader_(std::move(shader)), texture_(std::move(texture)), position_(position), frame_(frame),
      sheetWidth_(sheetWidth), sheetHeight_(sheetHeight),
      frameCols_(frameCols), frameRows_(frameRows)
{
    if (quadVAO_ == 0)
        initRenderData();

}

void Enemy::Draw(const glm::mat4& projection)
{
    // Calculate frame dimensions
    float frameWidth  = sheetWidth_  / static_cast<float>(frameCols_);
    float frameHeight = sheetHeight_ / static_cast<float>(frameRows_);

    // UV rectangle for this frame
    glm::vec2 uvSize   = { frameWidth / sheetWidth_, frameHeight / sheetHeight_ };
    glm::vec2 uvOffset = {
        frame_.x * uvSize.x,
        1.0f - (frame_.y + 1) * uvSize.y
    };

    // --- Properly center the sprite on its position ---
    // Current 'position_' is top-left corner; convert to center-based rendering
    glm::vec2 centerOffset(frameWidth * 0.5f, frameHeight * 0.5f);

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(position_ + centerOffset, 0.0f));
    model = glm::translate(model, glm::vec3(-centerOffset, 0.0f)); // keep origin top-left for UVs
    model = glm::scale(model, glm::vec3(frameWidth * manscale_, frameHeight * manscale_, 1.0f));

    shader_->Use();
    shader_->SetMatrix4("model", model);
    shader_->SetMatrix4("projection", projection);
    shader_->SetVector4f("uvRect", glm::vec4(uvOffset, uvSize));

    texture_->Bind();
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Enemy::Update(float dt,
                   const std::vector<const std::vector<std::vector<int> > *> &mapDataPtrs,
                   const std::unordered_set<int> &solidTiles,
                   int tileWidth, int tileHeight, const Circle &playerCircle) {
    float frameWidth = (sheetWidth_ / frameCols_) * manscale_;
    float frameHeight = (sheetHeight_ / frameRows_) * manscale_;
    float radius = 0.5f * glm::length(glm::vec2(frameWidth, frameHeight)) * collisionScale_;
Circle c = {
        position_ + glm::vec2(frameWidth, frameHeight) * 0.5f,
        0.5f * glm::length(glm::vec2(frameWidth, frameHeight)) * collisionScale_
    };

    // Compute map bounds from first collidable layer
    glm::vec2 bounds(0.0f);
    if (!mapDataPtrs.empty() && mapDataPtrs[0] && !mapDataPtrs[0]->empty()) {
        int rows = static_cast<int>(mapDataPtrs[0]->size());
        int cols = static_cast<int>((*mapDataPtrs[0])[0].size());
        bounds = { cols * tileWidth, rows * tileHeight };
    }
    if (!TryMoveCircle(c, velocity_, dt, bounds, mapDataPtrs, solidTiles, tileWidth, tileHeight)) {
         velocity_ = glm::vec2(0.0f);
     }

    // Back from center to top-left
    position_ = c.center - glm::vec2(frameWidth, frameHeight) * 0.5f;
}


void Enemy::SetFrame(glm::ivec2 frame) {
    frame_ = frame;
}

void Enemy::SetPosition(glm::vec2 position) {
    position_ = position;
}

void Enemy::SetScale(float manscale) {
    manscale_ = manscale;
}

void Enemy::initRenderData()
{
    float vertices[] = {
        0.0f, 1.0f,    0.0f, 1.0f,
        1.0f, 0.0f,    1.0f, 0.0f,
        0.0f, 0.0f,    0.0f, 0.0f,

        0.0f, 1.0f,    0.0f, 1.0f,
        1.0f, 1.0f,    1.0f, 1.0f,
        1.0f, 0.0f,    1.0f, 0.0f
    };

    glGenVertexArrays(1, &quadVAO_);
    glGenBuffers(1, &quadVBO_);

    glBindVertexArray(quadVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
Circle Enemy::ComputeBoundingCircle() const {
    float width  = (sheetWidth_ / frameCols_) * manscale_;
    float height = (sheetHeight_ / frameRows_) * manscale_;
    float radius = 0.5f * glm::length(glm::vec2(width, height))* collisionScale_;
    glm::vec2 center = position_ + glm::vec2(width, height) * 0.5f;
    return { center, radius };
}

glm::vec2 Enemy::GetHalfSize() const {
    float width  = (sheetWidth_ / frameCols_) * manscale_;
    float height = (sheetHeight_ / frameRows_) * manscale_;
    return { width * 0.5f, height * 0.5f };
}

void Enemy::SetCenter(const glm::vec2& center) {
    position_ = center - GetHalfSize();
}

Enemy::~Enemy() = default;
