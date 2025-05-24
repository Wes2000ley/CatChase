#include "DebugDraw.h"
#include <vector>
#include <glad/glad.h>

static unsigned int circleVAO = 0;
static unsigned int circleVBO = 0;
static unsigned int gridVAO = 0;
static unsigned int gridVBO = 0;

void InitDebugDraw() {
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
}

void DestroyDebugDraw() {
    if (circleVBO) glDeleteBuffers(1, &circleVBO);
    if (circleVAO) glDeleteVertexArrays(1, &circleVAO);
    if (gridVBO) glDeleteBuffers(1, &gridVBO);
    if (gridVAO) glDeleteVertexArrays(1, &gridVAO);
}

void DrawDebugCircle(const Circle& c, const glm::vec3& color, const glm::mat4& proj, std::shared_ptr<Shader> shader, int segments) {
    if (circleVAO == 0) InitDebugDraw();

    std::vector<glm::vec2> vertices;
    for (int i = 0; i <= segments; ++i) {
        float angle = glm::two_pi<float>() * i / segments;
        vertices.emplace_back(c.center + glm::vec2(cos(angle), sin(angle)) * c.radius);
    }

    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

    shader->Use();
    shader->SetMatrix4("projection", proj);
    shader->SetMatrix4("model", glm::mat4(1.0f));
    shader->SetVector3f("lineColor", color);

    glDrawArrays(GL_LINE_LOOP, 0, segments);
    glBindVertexArray(0);
}

void DrawDebugGrid(int cols, int rows, int tileWidth, int tileHeight, const glm::mat4& proj, std::shared_ptr<Shader> shader) {
    std::vector<float> lines;

    for (int x = 0; x <= cols; ++x) {
        float px = x * tileWidth;
        lines.push_back(px); lines.push_back(0.0f);
        lines.push_back(px); lines.push_back(rows * tileHeight);
    }

    for (int y = 0; y <= rows; ++y) {
        float py = y * tileHeight;
        lines.push_back(0.0f); lines.push_back(py);
        lines.push_back(cols * tileWidth); lines.push_back(py);
    }

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(float), lines.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    shader->Use();
    shader->SetMatrix4("projection", proj);
    shader->SetMatrix4("model", glm::mat4(1.0f));
    shader->SetVector3f("lineColor", glm::vec3(0.2f)); // Gray lines

    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size() / 2));
    glBindVertexArray(0);
}
