#include "Dog.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <unordered_set>
#include <array>
#include "Collision.h"

unsigned int Dog::quadVAO_ = 0;
unsigned int Dog::quadVBO_ = 0;

Dog::Dog(std::shared_ptr<Shader> shader,
         std::shared_ptr<Texture2D> texture,
         glm::vec2 position,
         glm::ivec2 frame)
    : shader_(std::move(shader)), texture_(std::move(texture)),
      position_(position), frame_(frame)
{
    if (quadVAO_ == 0)
        initRenderData();

    boundingBox_ = ComputeBoundingBox();
}

void Dog::Draw(const glm::mat4& projection)
{
    constexpr float sheetWidth = 256.0f;
    constexpr float sheetHeight = 48.0f;
    constexpr int frameCols = 16;
    constexpr int frameRows = 3;

    float frameWidth = sheetWidth / frameCols;
    float frameHeight = sheetHeight / frameRows;

    glm::vec2 uvSize = { frameWidth / sheetWidth, frameHeight / sheetHeight };
    glm::vec2 uvOffset = {
        frame_.x * uvSize.x,
        1.0f - (frame_.y + 1) * uvSize.y
    };

    //float baseOffsetDeg = 90.0f; // ✅ Use this if your sprite faces up
    // float baseOffsetDeg = 0.0f;  // Use this if it faces right
    // float baseOffsetDeg = 180.0f; // Use if it faces left
     float baseOffsetDeg = 270.0f; // Use if it faces down

    float angleDeg = static_cast<float>(facingDirection_) * 45.0f + baseOffsetDeg;    float angleRad = glm::radians(angleDeg);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position_, 0.0f));
    model = glm::translate(model, glm::vec3(0.5f * frameWidth * manscale_, 0.5f * frameHeight * manscale_, 0.0f));
    model = glm::rotate(model, angleRad, glm::vec3(0, 0, 1));
    model = glm::translate(model, glm::vec3(-0.5f * frameWidth * manscale_, -0.5f * frameHeight * manscale_, 0.0f));
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


void Dog::Update(
    float dt,
    const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
    const std::unordered_set<int>& solidTiles,
    int tileWidth,
    int tileHeight,
    glm::vec2 screenSize)
{
    float frameWidth  = (256.0f / 16.0f) * manscale_;
    float frameHeight = (48.0f / 3.0f) * manscale_;

    if (!TryMove(position_, velocity_, dt, frameWidth, frameHeight, screenSize, mapDataPtrs, solidTiles, tileWidth, tileHeight)) {
        velocity_ = glm::vec2(0.0f);
    }

    boundingBox_ = ComputeBoundingBox();
}



glm::vec4 Dog::ComputeBoundingBox() const {
    float frameWidth = (256.0f / 16.0f) * manscale_;
    float frameHeight = (48.0f / 3.0f) * manscale_;
    return { position_.x, position_.y, position_.x + frameWidth, position_.y + frameHeight };
}

glm::vec4 Dog::GetBoundingBox() const {
    return boundingBox_;
}

glm::vec2 Dog::GetPosition() const {
    return position_;
}

void Dog::SetPosition(const glm::vec2& pos) {
    position_ = pos;
    boundingBox_ = ComputeBoundingBox();
}

void Dog::SetScale(float scale) {
    manscale_ = scale;
}

void Dog::SetVelocity(glm::vec2 v) {
    velocity_ = v;

    if (v == glm::vec2(0.0f))
        return;

    float angle = glm::degrees(std::atan2(v.y, v.x));
    if (angle < 0) angle += 360.0f;

    facingDirection_ = static_cast<Direction8>(static_cast<int>((angle + 22.5f) / 45.0f) % 8);
}

void Dog::initRenderData()
{
    constexpr float vertices[] = {
        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 0.0f,  0.0f, 0.0f,
        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f
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
