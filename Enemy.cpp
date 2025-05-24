#include "Enemy.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "Dog.h"
#include "Dog.h"
#include "Enemies.h"
#include "EnemyRegistry.h"

unsigned int Enemy::quadVAO_ = 0;

Enemy::Enemy(Shader& shader, Texture2D& texture,
             glm::vec2 position, glm::ivec2 frame,
             float sheetWidth, float sheetHeight,
             int frameCols, int frameRows)
    : shader_(shader), texture_(texture), position_(position), frame_(frame),
      sheetWidth_(sheetWidth), sheetHeight_(sheetHeight),
      frameCols_(frameCols), frameRows_(frameRows)
{
    if (quadVAO_ == 0)
        initRenderData();
}

void Enemy::Draw(const glm::mat4& projection)
{
    // Frame size from spritesheet
    float frameWidth  = sheetWidth_  / static_cast<float>(frameCols_);
    float frameHeight = sheetHeight_ / static_cast<float>(frameRows_);

    // Texture coordinates
    glm::vec2 uvSize   = glm::vec2(frameWidth / sheetWidth_, frameHeight / sheetHeight_);
    glm::vec2 uvOffset = glm::vec2(
        frame_.x * uvSize.x,
        1.0f - (frame_.y + 1) * uvSize.y
    );

    // Transform: internal units (not screen-scaled)
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position_, 0.0f));
    model = glm::scale(model, glm::vec3(frameWidth * manscale_, frameHeight * manscale_, 1.0f));

    // Render
    shader_.Use();
    shader_.SetMatrix4("model", model);
    shader_.SetMatrix4("projection", projection);
    shader_.SetVector4f("uvRect", glm::vec4(uvOffset, uvSize));

    texture_.Bind();
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
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
        // positions   // tex coords
        0.0f, 1.0f,    0.0f, 1.0f,
        1.0f, 0.0f,    1.0f, 0.0f,
        0.0f, 0.0f,    0.0f, 0.0f,

        0.0f, 1.0f,    0.0f, 1.0f,
        1.0f, 1.0f,    1.0f, 1.0f,
        1.0f, 0.0f,    1.0f, 0.0f
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
