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


// Dog.cpp
Circle Dog::ComputeBoundingCircle() const {
    float frameWidth = (256.0f / 16.0f) * manscale_;
    float frameHeight = (48.0f / 3.0f) * manscale_;
    float radius = 0.5f * glm::length(glm::vec2(frameWidth, frameHeight)) * collisionScale_;
    glm::vec2 center = position_ + glm::vec2(frameWidth, frameHeight) * 0.5f;
    return { center, radius };
}

void Dog::Update(
    float dt,
    const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
    const std::unordered_set<int>& solidTiles,
    int tileWidth,
    int tileHeight,
    glm::vec2 screenSize)
{
    // ─── Begin: handle bite & cooldown timers ───
    // 1) If we're on cooldown (i.e. just finished a bite), reduce that timer:
    if (biteCooldown_ > 0.0f) {
        biteCooldown_ -= dt;
        if (biteCooldown_ < 0.0f)
            biteCooldown_ = 0.0f;
    }

    // 2) If currently biting, decrement the biteTimer_:
    if (isBiting_) {
        biteTimer_ -= dt;
        if (biteTimer_ <= 0.0f) {
            // Bite just finished → enter cooldown
            isBiting_ = false;
            biteCooldown_ = biteCooldownTime_;
            std::cout << "[Dog] Bite finished; entering cooldown.\n";
        }
    }
    // ─── End: bite & cooldown timers ───

    float frameW = (256.0f / 16.0f) * manscale_;
    float frameH = (48.0f / 3.0f) * manscale_;
    float radius = 0.5f * glm::length(glm::vec2(frameW, frameH)) * collisionScale_;
    Circle c = { position_ + glm::vec2(frameW, frameH) * 0.5f, radius };


    if (!TryMoveCircle(c, velocity_, dt, screenSize, mapDataPtrs, solidTiles, tileWidth, tileHeight)) {
        velocity_ = glm::vec2(0.0f);
    }

    // Back from center to top-left
    position_ = c.center - glm::vec2(frameW, frameH) * 0.5f;
}







glm::vec2 Dog::GetPosition() const {
    return position_;
}

void Dog::SetPosition(const glm::vec2& pos) {
    position_ = pos;
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
void Dog::SetCollisionScale(float scale) {
    collisionScale_ = scale;
}
void Dog::StartBite() {
    // Only allow a bite if we are not already biting and the cooldown has expired
    if (!isBiting_ && biteCooldown_ <= 0.0f) {
        isBiting_   = true;
        biteTimer_  = biteDuration_;
        // (You could also switch to a “bite” animation frame here by modifying frame_.x/frame_.y)
        std::cout << "[Dog] Starting bite!\n";
    }
}

Circle Dog::ComputeBiteCircle() const {
    // We want a small circle in front of the dog’s facing direction:
    // Pick a “bite range” of, say, 0.5 × the dog’s bounding circle radius,
    // and place it one radius‐and‐a‐bit in front of the dog’s center.
    Circle body = ComputeBoundingCircle();
    float biteRangeFactor = 0.5f;      // bite circle radius = 0.5 × body radius
    float offsetFactor    = 0.75f;     // how far in front of the body center we place it

    float biteRadius = body.radius * biteRangeFactor;
    float dx = 0.0f, dy = 0.0f;

switch (facingDirection_) {
        case Direction8::Right: dx = 1.0f;
            dy = 0.0f;
            break;
        case Direction8::DownRight: dx = 0.7071f;
            dy = -0.7071f;
            break;
        case Direction8::Down: dx = 0.0f;
            dy = -1.0f;
            break;
        case Direction8::DownLeft: dx = -0.7071f;
            dy = -0.7071f;
            break;
        case Direction8::Left: dx = -1.0f;
            dy = 0.0f;
            break;
        case Direction8::UpLeft: dx = -0.7071f;
            dy = 0.7071f;
            break;
        case Direction8::Up: dx = 0.0f;
            dy = 1.0f;
            break;
        case Direction8::UpRight: dx = 0.7071f;
            dy = 0.7071f;
            break;
    }

    glm::vec2 biteCenter = body.center + glm::vec2(dx, dy) * (body.radius * offsetFactor);
    return { biteCenter, biteRadius };
}
