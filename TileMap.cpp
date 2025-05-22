#include "TileMap.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Dog.h"
#include "Dog.h"

unsigned int TileMap::quadVAO_ = 0;

TileMap::TileMap(Shader& shader, Texture2D& tileset,
                 int textureWidth, int textureHeight,
                 int tileWidth, int tileHeight)
    : shader_(shader), tileset_(tileset),
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
    shader_.Use();
    shader_.SetMatrix4("projection", projection);
    tileset_.Bind();
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

            shader_.SetMatrix4("model", model);
            shader_.SetVector4f("uvRect", glm::vec4(uvOffset, uvSize));

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
