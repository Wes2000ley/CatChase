#pragma once

#include <functional>
#include <string>
#include <vector>

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

	void Render();
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


private:
	bool active_;
	int selectedIndex_;
	int selectedLevelIndex_ = 0;

	unsigned int quadVAO_ = 0;
	unsigned int quadVBO_ = 0;
	void initRenderData(); // new

	Mode currentMode_ = Mode::MAIN;
	std::vector<std::string> options_;
	std::vector<std::string> levelNames_;

};
