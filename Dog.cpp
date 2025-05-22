#include "Dog.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// Initialize static member
unsigned int Dog::quadVAO_ = 0;

Dog::Dog(Shader& shader, Texture2D& texture, glm::vec2 position, glm::ivec2 frame)
    : shader_(shader), texture_(texture), position_(position), frame_(frame)
{
    if (quadVAO_ == 0)
        initRenderData();
}

void Dog::Draw(const glm::mat4& projection, float scale)
{
    constexpr float sheetWidth  = 512.0f;
    constexpr float sheetHeight = 256.0f;
    constexpr int frameCols = 24;
    constexpr int frameRows = 8;

    float frameWidth  = sheetWidth  / static_cast<float>(frameCols);
    float frameHeight = sheetHeight / static_cast<float>(frameRows);

    glm::vec2 uvSize = glm::vec2(frameWidth / sheetWidth, frameHeight / sheetHeight);
    glm::vec2 uvOffset = glm::vec2(
        frame_.x * uvSize.x,
        1.0f - (frame_.y + 1) * uvSize.y
    );

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position_, 0.0f));
    model = glm::scale(model, glm::vec3(frameWidth * scale, frameHeight * scale, 1.0f));

    shader_.Use();
    shader_.SetMatrix4("model", model);
    shader_.SetMatrix4("projection", projection);
    shader_.SetVector4f("uvRect", glm::vec4(uvOffset, uvSize));

    texture_.Bind();
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Dog::initRenderData()
{
    std::cout << "[Dog] Initializing VAO/VBO...\n";

    float vertices[] = {
        // positions   // texture coords
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

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "[Dog] quadVAO_ = " << quadVAO_ << "\n";
}
