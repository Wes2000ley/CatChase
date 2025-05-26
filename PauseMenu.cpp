#include "PauseMenu.h"

#include <functional>
//#include <GL/gl.h>
#include <glad/glad.h>


#include "Collision.h"
#include "Collision.h"
#include "Collision.h"
#include "Collision.h"
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

void PauseMenu::Render(float screenWidth, float screenHeight) {
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    initRenderData();

    std::shared_ptr<Shader> shader = ResourceManager::GetShader("pause");
    shader->Use();
    shader->SetMatrix4("projection", glm::ortho(0.0f, screenWidth_, screenHeight_, 0.0f));
    shader->SetVector4f("overlayColor", glm::vec4(0.36f, 0.36f, 0.35f, 0.4f));
    shader->SetInteger("image", 0);

    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    TextRenderer& textTitle = ResourceManager::GetTextRenderer("pause");    // big title
    TextRenderer& textMenu  = ResourceManager::GetTextRenderer("default");  // smaller menu items
    glm::mat4 projection = glm::ortho(0.0f, screenWidth_, screenHeight_, 0.0f);
    float centerX = screenWidth_ / 2.0f;
    float scaleTitle = 2.0f;
    float scaleOption = 1.5f;
    if (currentMode_ == Mode::MAIN) {
        std::string title = "Paused";
        float titleWidth = textTitle.MeasureTextWidth(title, scaleTitle);
        float titleX = centerX - titleWidth / 2.0f;
        float titleY = screenHeight_ / 2.0f - 150.0f;

        // Use title font for the title
        RenderOption(title, titleX, titleY, scaleTitle, glm::vec3(1.0f), projection, "pause");

        float spacing = 60.0f;
        float optionYStart = titleY + 100.0f;

        for (int i = 0; i < COUNT; ++i) {
            const std::string& label = options_[i];
            float optionWidth = textMenu.MeasureTextWidth(label, scaleOption);
            float optionHeight = 30.0f * scaleOption;

            float x = centerX - optionWidth / 2.0f;
            float y = optionYStart + i * spacing;

            glm::vec3 color = (i == hoveredIndex_) ? glm::vec3(0.0f) : glm::vec3(1.0f);
            RenderOption(label, x, y, scaleOption, color, projection, "default");
            // if (i == selectedIndex_) {
            //     auto bounds = GetOptionBounds(i, screenWidth_, screenHeight_, scaleOption);
            //     RenderSelectionBox(bounds.x, bounds.y, bounds.width, bounds.height, glm::vec4(1.0f, 0.0f, 0.0f, 0.3f), projection);
            // }
        }
    }
    else if (currentMode_ == Mode::LEVEL_SELECT) {
        std::string header = "Select Level:";
        float headerWidth = textTitle.MeasureTextWidth(header, scaleOption);
        float headerY = screenHeight_ / 2.0f - 150.0f;

        // Use title font for the header
        RenderOption(header, centerX - headerWidth / 2.0f, headerY, scaleOption, glm::vec3(1.0f), projection, "pause");

        float spacing = 60.0f;
        float optionYStart = headerY + 100.0f;

        levelOptionPositions_.clear();

        for (int i = 0; i < (int)levelNames_.size(); ++i) {
            const std::string& label = levelNames_[i];
            float optionWidth = textMenu.MeasureTextWidth(label, scaleOption);
            float optionHeight = 30.0f * scaleOption;

            float x = centerX - optionWidth / 2.0f;
            float y = optionYStart + i * spacing;

            levelOptionPositions_.emplace_back(x, y);

            glm::vec3 color = (i == hoveredLevelIndex_) ? glm::vec3(0.0f) : glm::vec3(1.0f);
            RenderOption(label, x, y, scaleOption, color, projection, "default");
            // if (i == selectedLevelIndex_) {
            //     auto bounds = GetLevelBounds(i, screenWidth_, screenHeight_, scaleOption);
            //     RenderSelectionBox(bounds.x, bounds.y, bounds.width, bounds.height, glm::vec4(1.0f, 0.0f, 0.0f, 0.3f), projection);
            // }

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
PauseMenu::MenuOptionBounds PauseMenu::GetOptionBounds(int index, float screenWidth, float screenHeight,
                                                       float scale) const {
    TextRenderer &text = ResourceManager::GetTextRenderer("default");
    std::string label = GetOptionLabel(index);

    float centerX = screenWidth / 2.0f;
    float spacing = 60.0f;
    float titleY = screenHeight / 2.0f - 150.0f;
    float optionYStart = titleY + 100.0f;
    float y = optionYStart + index * spacing;

    glm::vec4 bounds = text.MeasureRenderedTextBounds(
        label,
        centerX - text.MeasureTextWidth(label, scale) / 2.0f,
        y,
        scale
    );

    float padX = bounds.z * SELECTION_PADDING_X;
    float padY = bounds.w * SELECTION_PADDING_Y;

    return {
        bounds.x - padX,
        bounds.y - padY,
        bounds.z + padX * 2.0f,
        bounds.w + padY * 2.0f
    };
}


PauseMenu::MenuOptionBounds PauseMenu::GetLevelBounds(int index, float screenWidth, float screenHeight, float scale) const {
    if (index < 0 || index >= (int)levelOptionPositions_.size())
        return {0, 0, 0, 0};

    TextRenderer& text = ResourceManager::GetTextRenderer("default");
    const std::string& label = levelNames_[index];
    glm::vec2 pos = levelOptionPositions_[index];

    glm::vec4 bounds = text.MeasureRenderedTextBounds(label, pos.x, pos.y, scale);

    float padX = bounds.z * SELECTION_PADDING_X;
    float padY = bounds.w * SELECTION_PADDING_Y;

    return {
        bounds.x - padX,
        bounds.y - padY,
        bounds.z + padX * 2.0f,
        bounds.w + padY * 2.0f
    };
}



void PauseMenu::NavigateLevels(int direction) {
    if (levelNames_.empty()) return;
    selectedLevelIndex_ = (selectedLevelIndex_ + direction + levelNames_.size()) % levelNames_.size();
}
void PauseMenu::initRenderData() {
    if (quadVAO_ == 0) {
        float vertices[] = {
            0.0f, screenHeight_, 0.0f, 1.0f,
            0.0f, 0.0f,          0.0f, 0.0f,
            screenWidth_, 0.0f, 1.0f, 0.0f,

            0.0f, screenHeight_, 0.0f, 1.0f,
            screenWidth_, 0.0f, 1.0f, 0.0f,
            screenWidth_, screenHeight_, 1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO_);
        glGenBuffers(1, &quadVBO_);
        glBindVertexArray(quadVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    if (boxVAO_ == 0) {
        glGenVertexArrays(1, &boxVAO_);
        glGenBuffers(1, &boxVBO_);
        glBindVertexArray(boxVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, boxVBO_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 6, nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
PauseMenu::~PauseMenu() {
    if (quadVAO_) glDeleteVertexArrays(1, &quadVAO_);
    if (quadVBO_) glDeleteBuffers(1, &quadVBO_);

}
void PauseMenu::RenderOption(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& proj, const std::string& rendererName) {
    TextRenderer& textRenderer = ResourceManager::GetTextRenderer(rendererName);
    textRenderer.RenderText(text, x, y, scale, color, proj);
}



void PauseMenu::RenderSelectionBox(float x, float y, float width, float height, const glm::vec4& color, const glm::mat4& proj) {
    struct BoxVertex { float x, y; };
    BoxVertex vertices[6] = {
        {x, y + height}, {x, y}, {x + width, y},
        {x, y + height}, {x + width, y}, {x + width, y + height}
    };

    glBindVertexArray(boxVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    auto shader = ResourceManager::GetShader("box");
    shader->Use();
    shader->SetMatrix4("projection", proj);
    shader->SetVector4f("color", color);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
void PauseMenu::OnMouseMove(float mx, float my) {
    hoveredIndex_ = -1;
    hoveredLevelIndex_ = -1;

    if (currentMode_ == Mode::MAIN) {
        for (int i = 0; i < COUNT; ++i) {
            auto bounds = GetOptionBounds(i, screenWidth_, screenHeight_, 1.5f);
            if (mx >= bounds.x && mx <= bounds.x + bounds.width &&
                my >= bounds.y && my <= bounds.y + bounds.height) {
                hoveredIndex_ = i;
                break;
                }
        }
    } else if (currentMode_ == Mode::LEVEL_SELECT) {
        for (int i = 0; i < (int)levelNames_.size(); ++i) {
            auto bounds = GetLevelBounds(i, screenWidth_, screenHeight_, 1.5f);
            if (mx >= bounds.x && mx <= bounds.x + bounds.width &&
                my >= bounds.y && my <= bounds.y + bounds.height) {
                hoveredLevelIndex_ = i;
                break;
                }
        }
    }
}

bool PauseMenu::HandleInput(GLFWwindow* window, float dt,
                            const std::function<void(Option)>& onMainSelect,
                            const std::function<void(int)>& onLevelSelect)
{
    static bool mousePressed = false;
    static bool enterPressed = false;

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    OnMouseMove(mx, my);

    float scaleOption = 1.5f;

    if (IsInLevelSelectMode()) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mousePressed) {
            for (int i = 0; i < (int)levelNames_.size(); ++i) {
                auto bounds = GetLevelBounds(i, screenWidth_, screenHeight_, scaleOption);
                if (mx >= bounds.x && mx <= bounds.x + bounds.width &&
                    my >= bounds.y && my <= bounds.y + bounds.height) {
                    if (onLevelSelect) onLevelSelect(i);
                    break;
                }
            }
            mousePressed = true;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
            mousePressed = false;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            NavigateLevels(-1);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            NavigateLevels(+1);

        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterPressed) {
            if (onLevelSelect) onLevelSelect(selectedLevelIndex_);
            enterPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE)
            enterPressed = false;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            ExitLevelSelect();

        return true;
    }

    // Main Menu mode
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mousePressed) {
        for (int i = 0; i < COUNT; ++i) {
            auto bounds = GetOptionBounds(i, screenWidth_, screenHeight_, scaleOption);
            if (mx >= bounds.x && mx <= bounds.x + bounds.width &&
                my >= bounds.y && my <= bounds.y + bounds.height) {
                if (i == CHANGE_LEVEL) {
                    currentMode_ = Mode::LEVEL_SELECT;
                } else if (onMainSelect) {
                    onMainSelect(static_cast<Option>(i));
                }
                break;
            }
        }
        mousePressed = true;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
        mousePressed = false;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        Navigate(-1);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        Navigate(+1);

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterPressed) {
        Select([&](Option opt) {
            if (onMainSelect) onMainSelect(opt);
        });
        enterPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE)
        enterPressed = false;

    return true;
}
