#pragma once

#include <functional>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class PauseMenu {
public:
	enum Option { RESUME, CHANGE_LEVEL, QUIT, COUNT };
	struct MenuOptionBounds { float x, y, width, height; };
	enum class Mode {
		MAIN,
		LEVEL_SELECT
	};

	PauseMenu();
	~PauseMenu();

	void SetActive(bool active);
	bool IsActive() const;

	void Navigate(int direction);
	void Select(const std::function<void(Option)> &callback);
	void SetSelectedIndex(int index);
	int GetSelectedIndex() const { return selectedIndex_; }

	void Render(float screenWidth, float screenHeight); // replaces hardcoded size version
	std::string GetOptionLabel(int index) const;
	float GetOptionY(int index, float screenHeight) const;
	MenuOptionBounds GetOptionBounds(int index, float screenWidth, float screenHeight, float scale) const;

	// Level selection
	void SetLevels(const std::vector<std::string>& levels) { levelNames_ = levels; }
	int GetSelectedLevel() const { return selectedLevelIndex_; }
	void NavigateLevels(int direction);
	bool IsInLevelSelectMode() const { return currentMode_ == Mode::LEVEL_SELECT; }
	void ExitLevelSelect() { currentMode_ = Mode::MAIN; }
	MenuOptionBounds GetLevelBounds(int index, float screenWidth, float screenHeight, float scale) const;

	int GetLevelCount() const { return static_cast<int>(levelNames_.size()); }
	void SetSelectedLevel(int index) { selectedLevelIndex_ = index; }
	void OnMouseMove(float x, float y);
	void OnMouseClick(float x, float y);
	bool HandleInput(GLFWwindow* window, float dt,
				 const std::function<void(Option)>& onMainSelect,
				 const std::function<void(int)>& onLevelSelect);


private:
	bool active_;
	int selectedIndex_;
	int selectedLevelIndex_ = 0;


	Mode currentMode_ = Mode::MAIN;
	std::vector<std::string> options_;
	std::vector<std::string> levelNames_;
	void RenderOption(const ::std::string &text, float x, float y, float scale, const glm::vec3 & color, const glm::mat4 & proj, const std::
	                  string &rendererName);
	void RenderSelectionBox(float x, float y, float width, float height, const glm::vec4& color, const glm::mat4& proj);

	void initRenderData(); // now initializes box quad too

	GLuint quadVAO_ = 0, quadVBO_ = 0;
	GLuint boxVAO_ = 0, boxVBO_ = 0;

	float screenWidth_ = 1920.0f;
	float screenHeight_ = 1080.0f;
	std::vector<glm::vec2> levelOptionPositions_;

};
