#pragma once
#include <vector>
#include <string>
#include <functional>

#include <glm/glm.hpp>

class Shader;
class TextRenderer;

class PauseMenu {
public:
	enum Option {
		RESUME,
		CHANGE_LEVEL,
		QUIT,
		COUNT
	};

	PauseMenu();
	struct MenuOptionBounds {
		float x, y, width, height;
	};
	void SetActive(bool active);
	bool IsActive() const;

	void Navigate(int direction); // -1 or +1
	void Select(std::function<void(Option)> callback);
	void Render();
	void SetSelectedIndex(int index);
	std::string GetOptionLabel(int index) const;
	float GetOptionY(int index, float screenHeight) const;
	MenuOptionBounds GetOptionBounds(int index, float screenWidth, float screenHeight, float scale) const;


private:
	bool active_;
	int selectedIndex_;
	std::vector<std::string> options_;
};
