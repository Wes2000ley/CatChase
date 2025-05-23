#include "PauseMenu.h"
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

void PauseMenu::Select(std::function<void(Option)> callback) {
    if (callback) callback(static_cast<Option>(selectedIndex_));
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

    Shader shader = ResourceManager::GetShader("pause");
    shader.Use();
    shader.SetMatrix4("projection", glm::ortho(0.0f, w, h, 0.0f));
    shader.SetVector4f("overlayColor", glm::vec4(0.36f, 0.36f, 0.35f, 0.4f));
    shader.SetInteger("image", 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    float vertices[] = {
        0.0f, h, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        w,    0.0f, 1.0f, 0.0f,

        0.0f, h, 0.0f, 1.0f,
        w,    0.0f, 1.0f, 0.0f,
        w,    h, 1.0f, 1.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    TextRenderer& text = ResourceManager::GetTextRenderer("default");
    glm::mat4 projection = glm::ortho(0.0f, w, h, 0.0f);
    float centerX = w / 2.0f;
    float scaleTitle = 2.0f;
    float scaleOption = 1.5f;

    std::string title = "Paused";
    float titleWidth = text.MeasureTextWidth(title, scaleTitle);
    float titleX = centerX - titleWidth / 2.0f;
    float titleY = h / 2.0f - 120.0f;
    text.RenderText(title, titleX, titleY, scaleTitle, glm::vec3(1.0f), projection);

    float startY = titleY + 80.0f;
    for (int i = 0; i < COUNT; ++i) {
        std::string label = options_[i];
        float optionWidth = text.MeasureTextWidth(label, scaleOption);
        float x = centerX - optionWidth / 2.0f;
        float y = startY + i * 50.0f;
        glm::vec3 color = (i == selectedIndex_) ? glm::vec3(1.0f, 1.0f, 0.3f) : glm::vec3(1.0f);
        text.RenderText(label, x, y, scaleOption, color, projection);
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
