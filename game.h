#pragma once

#include <array>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Represents the current state of the game
enum class GameState {
	Active,
	Menu,
	Win
};

// Game class holds all game-related state and functionality.
class Game
{
public:
	Game(unsigned int width, unsigned int height);
	~Game();

	void Init();
	void ProcessInput(float dt);
	void Update(float dt);
	void Render();

	// Read-only accessors for encapsulated state
	[[nodiscard]] unsigned int GetWidth() const { return Width; }
	[[nodiscard]] unsigned int GetHeight() const { return Height; }
	[[nodiscard]] GameState GetState() const { return State; }

	// Expose mutable input state safely
	bool* GetKeys() { return Keys.data(); }

private:
	GameState State = GameState::Active;
	std::array<bool, 1024> Keys{}; // zero-initialized
	unsigned int Width = 0;
	unsigned int Height = 0;
};
