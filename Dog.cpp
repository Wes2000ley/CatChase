#include "Dog.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <unordered_set>
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

    boundingBox_ = ComputeBoundingBox(); // initialize box
}

void Dog::Draw(const glm::mat4& projection)
{
    constexpr float sheetWidth  = 256.0f;
    constexpr float sheetHeight = 48.0f;
    constexpr int frameCols = 16;
    constexpr int frameRows = 3;

    float frameWidth  = sheetWidth  / static_cast<float>(frameCols);
    float frameHeight = sheetHeight / static_cast<float>(frameRows);

    glm::vec2 uvSize = glm::vec2(frameWidth / sheetWidth, frameHeight / sheetHeight);
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

void Dog::Update(float dt,
                 const std::vector<std::unique_ptr<TileMap>>& layers,
                 const std::unordered_set<int>& solidTiles,
                 glm::vec2 screenSize)
{
    glm::vec2 newPos = position_ + velocity_ * dt;

    float frameWidth  = (256.0f / 16.0f) * manscale_;
    float frameHeight = (48.0f / 3.0f) * manscale_;
    glm::vec2 topLeft     = newPos;
    glm::vec2 bottomRight = newPos + glm::vec2(frameWidth, frameHeight);

    // Edge bounds
    if (topLeft.x < 0.0f || topLeft.y < 0.0f ||
        bottomRight.x > screenSize.x || bottomRight.y > screenSize.y) {
        std::cout << "📏 Edge collision\n";
        velocity_ = glm::vec2(0.0f);
        return;
    }

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
                    std::cout << "🧱 COLLISION at (" << x << "," << y << ") ID: " << tileID << " [layer " << i << "]\n";
                    velocity_ = glm::vec2(0.0f);
                    return;
                }
            }
        }
    }

    // ✅ Apply position and update bounding box
    position_ = newPos;
    boundingBox_ = ComputeBoundingBox();
}

glm::vec4 Dog::ComputeBoundingBox() const {
    float frameWidth  = (256.0f / 16.0f) * manscale_;
    float frameHeight = (48.0f / 3.0f) * manscale_;
    glm::vec2 bottomRight = position_ + glm::vec2(frameWidth, frameHeight);
    return glm::vec4(position_.x, position_.y, bottomRight.x, bottomRight.y);
}

glm::vec4 Dog::GetBoundingBox() const {
    return boundingBox_;
}

glm::vec2 Dog::GetPosition() const {
    return position_;
}

void Dog::SetPosition(const glm::vec2& pos) {
    position_ = pos;
    boundingBox_ = ComputeBoundingBox(); // keep updated
}

void Dog::SetScale(float manscale) {
    manscale_ = manscale;
}

void Dog::initRenderData()
{
    std::cout << "[Dog] Initializing VAO/VBO...\n";

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

    std::cout << "[Dog] quadVAO_ = " << quadVAO_ << "\n";
}
