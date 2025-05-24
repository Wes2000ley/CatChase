#include "Enemy.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <utility>

#include "TileMap.h"

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

    boundingBox_ = ComputeBoundingBox(); // initial
}

void Enemy::Draw(const glm::mat4& projection)
{
    float frameWidth  = sheetWidth_  / static_cast<float>(frameCols_);
    float frameHeight = sheetHeight_ / static_cast<float>(frameRows_);

    glm::vec2 uvSize   = glm::vec2(frameWidth / sheetWidth_, frameHeight / sheetHeight_);
    glm::vec2 uvOffset = glm::vec2(
        frame_.x * uvSize.x,
        1.0f - (frame_.y + 1) * uvSize.y
    );

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position_, 0.0f));
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
                   const std::vector<std::unique_ptr<TileMap>>& layers,
                   const std::unordered_set<int>& solidTiles)
{
    glm::vec2 newPos = position_ + velocity_ * dt;

    float frameWidth  = (sheetWidth_ / frameCols_) * manscale_;
    float frameHeight = (sheetHeight_ / frameRows_) * manscale_;

    glm::vec2 topLeft     = newPos;
    glm::vec2 bottomRight = newPos + glm::vec2(frameWidth, frameHeight);

    if (layers.empty() || layers[0]->GetMapData().empty())
        return;

    int tileWidth  = layers[0]->GetTileWidth();
    int tileHeight = layers[0]->GetTileHeight();

    int tileX1 = static_cast<int>(topLeft.x) / tileWidth;
    int tileY1 = static_cast<int>(topLeft.y) / tileHeight;
    int tileX2 = static_cast<int>(bottomRight.x) / tileWidth;
    int tileY2 = static_cast<int>(bottomRight.y) / tileHeight;

    for (int y = tileY1; y <= tileY2; ++y) {
        for (int x = tileX1; x <= tileX2; ++x) {
            for (size_t i = 0; i < layers.size(); ++i) {
                const auto& mapData = layers[i]->GetMapData();
                if (y < 0 || y >= static_cast<int>(mapData.size()) ||
                    x < 0 || x >= static_cast<int>(mapData[0].size()))
                    continue;

                int tileID = mapData[y][x];
                if (solidTiles.count(tileID)) {
                    std::cout << "💢 Enemy COLLISION at (" << x << "," << y << ") ID: " << tileID << " [layer " << i << "]\n";
                    velocity_ = glm::vec2(0.0f);
                    return;
                }
            }
        }
    }

    position_ = newPos;
    boundingBox_ = ComputeBoundingBox(); // update cached AABB
}

glm::vec4 Enemy::ComputeBoundingBox() const {
    float width  = (sheetWidth_ / frameCols_) * manscale_;
    float height = (sheetHeight_ / frameRows_) * manscale_;
    return glm::vec4(position_.x, position_.y, position_.x + width, position_.y + height);
}

glm::vec4 Enemy::GetBoundingBox() const {
    return boundingBox_;
}

void Enemy::SetFrame(glm::ivec2 frame) {
    frame_ = frame;
}

void Enemy::SetPosition(glm::vec2 position) {
    position_ = position;
    boundingBox_ = ComputeBoundingBox();
}

void Enemy::SetScale(float manscale) {
    manscale_ = manscale;
    boundingBox_ = ComputeBoundingBox();
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

Enemy::~Enemy() = default;
