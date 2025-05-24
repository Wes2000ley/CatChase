#include "TileMap.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>


#include "Dog.h"
#include "Dog.h"
#include "RESOURCE_MANAGER.h"

unsigned int TileMap::quadVAO_ = 0;

TileMap::TileMap(std::shared_ptr<Shader> shader, std::shared_ptr<Texture2D> tileset,
                 int textureWidth, int textureHeight,
                 int tileWidth, int tileHeight)
    : shader_(std::move(shader)), tileset_(std::move(tileset)),
      tileWidth_(tileWidth), tileHeight_(tileHeight)
{
    tilesPerRow_ = textureWidth / tileWidth_;
    tilesPerCol_ = textureHeight / tileHeight_;

    if (quadVAO_ == 0)
        initRenderData();
}


void TileMap::Load(const std::vector<std::vector<int>>& mapData) {
    mapData_ = mapData;
}

void TileMap::Draw(const glm::mat4& projection)
{
    shader_->Use();
    shader_->SetMatrix4("projection", projection);
    tileset_->Bind();
    glBindVertexArray(quadVAO_);

    int mapCols = mapData_[0].size();
    int mapRows = mapData_.size();

    glm::vec2 uvSize(1.0f / tilesPerRow_, 1.0f / tilesPerCol_);

    for (int y = 0; y < mapRows; ++y) {
        for (int x = 0; x < mapCols; ++x) {
            int tileID = mapData_[y][x];
            if (tileID < 0) continue;

            int tu = tileID % tilesPerRow_;
            int tv = tilesPerCol_ - 1 - (tileID / tilesPerRow_);

            glm::vec2 uvOffset(tu * uvSize.x, 1.0f - (tv + 1) * uvSize.y);

            glm::vec3 pos(x * tileWidth_, y * tileHeight_, 0.0f);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
            model = glm::scale(model, glm::vec3(tileWidth_, tileHeight_, 1.0f));

            shader_->SetMatrix4("model", model);
            shader_->SetVector4f("uvRect", glm::vec4(uvOffset, uvSize));

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    glBindVertexArray(0);
}

void TileMap::initRenderData() const {
    float vertices[] = {
        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 0.0f,  0.0f, 0.0f,

        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f
    };

    unsigned int VBO;
    glGenVertexArrays(1, &quadVAO_);
    glGenBuffers(1, &VBO);

    glBindVertexArray(quadVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TileMap::initGridLines() {
    if (gridVAO_ != 0) return;  // Already initialized

    int cols = mapData_[0].size();
    int rows = mapData_.size();

    // Generate grid lines (horizontal and vertical)
    for (int x = 0; x <= cols; ++x) {
        float px = x * tileWidth_;
        gridLines_.push_back(px); gridLines_.push_back(0.0f);
        gridLines_.push_back(px); gridLines_.push_back(rows * tileHeight_);
    }
    for (int y = 0; y <= rows; ++y) {
        float py = y * tileHeight_;
        gridLines_.push_back(0.0f); gridLines_.push_back(py);
        gridLines_.push_back(cols * tileWidth_); gridLines_.push_back(py);
    }

    // Upload to GPU
    glGenVertexArrays(1, &gridVAO_);
    glGenBuffers(1, &gridVBO_);

    glBindVertexArray(gridVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO_);
    glBufferData(GL_ARRAY_BUFFER, gridLines_.size() * sizeof(float), gridLines_.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void TileMap::DrawDebugGrid(const glm::mat4& projection)
{
    initGridLines(); // Ensure it's set up

    // 1. Draw grid lines
    std::shared_ptr<Shader> gridShader = ResourceManager::GetShader("grid");
    gridShader->Use();
    gridShader->SetMatrix4("projection", projection);
    gridShader->SetMatrix4("model", glm::mat4(1.0f));
    gridShader->SetVector3f("lineColor", glm::vec3(0.0f)); // Black

    glBindVertexArray(gridVAO_);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(gridLines_.size() / 2));
    glBindVertexArray(0);

    // 2. Draw coordinates inside each tile
    if (!textRenderer_) return; // Avoid null crash

    std::shared_ptr<Shader> textShader = ResourceManager::GetShader("text");

    const int cols = static_cast<int>(mapData_[0].size());
    const int rows = static_cast<int>(mapData_.size());

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            std::string label = std::to_string(x) + "," + std::to_string(y);
            float xpos = x * tileWidth_ + 2.0f;
            float ypos = y * tileHeight_ + 12.0f;
            textRenderer_->RenderText(label, xpos, ypos, 0.25f, glm::vec3(1.0f), projection); // ✅ Use dynamic label
        }
    }
}
