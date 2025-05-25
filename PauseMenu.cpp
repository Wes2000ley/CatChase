#include "PauseMenu.h"

#include <functional>
//#include <GL/gl.h>
//#include <glad/glad.h>

#include <glad/glad.h>

#include "RESOURCE_MANAGER.h"
#include "TEXT_RENDERER.h"

PauseMenu::PauseMenu()
    : active_(false), selectedIndex_(0),
      options_({"Resume", "Change Level", "Quit"})
{ }


void PauseMenu::SetActive(bool active) { active_ = active; }
bool PauseMenu::IsActive() const { return active_; }

void PauseMenu::Navigate(int direction) {
    selectedIndex_ = (selectedIndex_ + direction + COUNT) % COUNT;
}

void PauseMenu::Select(const std::function<void(Option)> &callback) {
    if (currentMode_ == Mode::MAIN) {
        if (selectedIndex_ == CHANGE_LEVEL) {
            currentMode_ = Mode::LEVEL_SELECT;
            // DO NOT call callback here!
        } else if (callback) {
            callback(static_cast<Option>(selectedIndex_));
        }
    }
}



void PauseMenu::SetSelectedIndex(int index) {
    if (index >= 0 && index < COUNT)
        selectedIndex_ = index;
}

void PauseMenu::Render() {
    float w = 1920.0f, h = 1080.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    std::shared_ptr<Shader> shader = ResourceManager::GetShader("pause");
    shader->Use();
    shader->SetMatrix4("projection", glm::ortho(0.0f, w, h, 0.0f));
    shader->SetVector4f("overlayColor", glm::vec4(0.36f, 0.36f, 0.35f, 0.4f));
    shader->SetInteger("image", 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    initRenderData(); // ✅ new

    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    TextRenderer& text = ResourceManager::GetTextRenderer("default");
    glm::mat4 projection = glm::ortho(0.0f, w, h, 0.0f);
    float centerX = w / 2.0f;
    float scaleTitle = 2.0f;
    float scaleOption = 1.5f;

    float startY = 0.0f;

    if (currentMode_ == Mode::MAIN) {
        std::string title = "Paused";
        float titleWidth = text.MeasureTextWidth(title, scaleTitle);
        float titleX = centerX - titleWidth / 2.0f;
        float titleY = h / 2.0f - 120.0f;
        text.RenderText(title, titleX, titleY, scaleTitle, glm::vec3(1.0f), projection);
        startY = titleY + 80.0f;
    } else if (currentMode_ == Mode::LEVEL_SELECT) {
        float headerY = h / 2.0f - 120.0f;
        startY = headerY + 60.0f;
    }

    if (currentMode_ == Mode::MAIN) {
        for (int i = 0; i < COUNT; ++i) {
            const std::string& label = options_[i];
            float optionWidth = text.MeasureTextWidth(label, scaleOption);
            float x = centerX - optionWidth / 2.0f;
            float y = startY + i * 50.0f;
            glm::vec3 color = (i == selectedIndex_) ? glm::vec3(1.0f, 1.0f, 0.3f) : glm::vec3(1.0f);
            text.RenderText(label, x, y, scaleOption, color, projection);
        }
    } else if (currentMode_ == Mode::LEVEL_SELECT) {
        std::string header = "Select Level:";
        float headerWidth = text.MeasureTextWidth(header, scaleOption);
        float headerY = h / 2.0f - 120.0f;
        text.RenderText(header, centerX - headerWidth / 2.0f, headerY, scaleOption, glm::vec3(1.0f), projection);

        for (int i = 0; i < (int) levelNames_.size(); ++i) {
            const std::string& label = levelNames_[i];
            float optionWidth = text.MeasureTextWidth(label, scaleOption);
            float x = centerX - optionWidth / 2.0f;
            float y = headerY + 60.0f + i * (scaleOption * 35.0f);
            glm::vec3 color = (i == selectedLevelIndex_) ? glm::vec3(0.5f, 1.0f, 0.5f) : glm::vec3(1.0f);
            text.RenderText(label, x, y, scaleOption, color, projection);
        }
    }
}

std::string PauseMenu::GetOptionLabel(int index) const {
    if (index >= 0 && index < COUNT)
        return options_[index];
    return "";
}

    float PauseMenu::GetOptionY(int index, float screenHeight) const {
    float titleY = screenHeight / 2.0f - 120.0f;
    float startY = titleY + 80.0f;
    return startY + index * 50.0f;  // 50 = lineSpacing
    
}
PauseMenu::MenuOptionBounds PauseMenu::GetOptionBounds(int index, float screenWidth, float screenHeight, float scale) const {
    TextRenderer& text = ResourceManager::GetTextRenderer("default");
    std::string label = GetOptionLabel(index);

    float optionWidth = text.MeasureTextWidth(label, scale);
    float x = screenWidth / 2.0f - optionWidth / 2.0f;
    float y = GetOptionY(index, screenHeight);

    float optionHeight = 24.0f * scale; // Approximate height per your font size and scale

    // Adjust y up so the box includes text baseline offset
    y -= optionHeight * 0.7f; // tweak for alignment

    return { x, y, optionWidth, optionHeight };
}
PauseMenu::MenuOptionBounds PauseMenu::GetLevelBounds(int index, float w, float h, float scale) const {
    TextRenderer& text = ResourceManager::GetTextRenderer("default");
    std::string label = levelNames_[index];

    float width = text.MeasureTextWidth(label, scale);
    float height = 24.0f * scale;
    float centerX = w / 2.0f;
    float headerY = h / 2.0f - 120.0f;
    float y = headerY + 60.0f + index * (scale * 35.0f);

    return { centerX - width / 2.0f, y - height * 0.7f, width, height };
}
void PauseMenu::NavigateLevels(int direction) {
    if (levelNames_.empty()) return;
    selectedLevelIndex_ = (selectedLevelIndex_ + direction + levelNames_.size()) % levelNames_.size();
}
void PauseMenu::initRenderData() {
    if (quadVAO_ != 0)
        return;

    float vertices[] = {
        0.0f, 1080.0f, 0.0f, 1.0f,
        0.0f,    0.0f, 0.0f, 0.0f,
        1920.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 1080.0f, 0.0f, 1.0f,
        1920.0f, 0.0f, 1.0f, 0.0f,
        1920.0f, 1080.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO_);
    glGenBuffers(1, &quadVBO_);

    glBindVertexArray(quadVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

PauseMenu::~PauseMenu() {
    if (quadVAO_) glDeleteVertexArrays(1, &quadVAO_);
    if (quadVBO_) glDeleteBuffers(1, &quadVBO_);

}
