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
void PauseMenu::Render() {
	// Draw translucent background
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	Shader shader = ResourceManager::GetShader("sprite");
	shader.Use();
	shader.SetVector4f("spriteColor", glm::vec4(0.0f, 0.0f, 0.0f, 0.6f));
	shader.SetMatrix4("model", glm::mat4(1.0f));
	shader.SetInteger("image", 0);

	float w = 1920.0f, h = 1080.0f;

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
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// Text rendering
	TextRenderer& text = ResourceManager::GetTextRenderer("default");

	glm::mat4 projection = glm::ortho(0.0f, w, h, 0.0f);
	float centerX = w / 2.0f;
	float y = h / 2.0f;
	float scale = 1.0f;

	// Title
	text.RenderText("Paused", centerX - 100.0f, y - 150.0f, 1.5f, glm::vec3(1.0f), projection);

	// Menu options
	for (int i = 0; i < COUNT; ++i) {
		glm::vec3 color = (i == selectedIndex_) ? glm::vec3(1.0f, 1.0f, 0.3f) : glm::vec3(1.0f);
		text.RenderText(options_[i], centerX - 80.0f, y + i * 40.0f, scale, color, projection);
	}
}
