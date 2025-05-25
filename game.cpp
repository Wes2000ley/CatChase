#include "game.h"

#include <unordered_set>
#include <cstdlib> // for rand()
#include <ctime>   // for seeding rand (e.g., in main())

#include "Dog.h"
#include "Enemies.h"
#include "Enemy.h"
#include "RESOURCE_MANAGER.h"
#include "TileMap.h"
#include "Collision.h"
#include "LevelManager.h"
#include "TEXT_RENDERER.h"


#include "PauseMenu.h"
PauseMenu pauseMenu;
bool isPaused = false;




Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
    ResourceManager::UnloadShader("pause");
    // OR
    ResourceManager::Clear(); // unloads ALL shaders/textures
}

void Game::Init() {

    srand(static_cast<unsigned>(time(nullptr)));
    ResourceManager::LoadShader("resources/shaders/pause.vert", "resources/shaders/pause.frag", nullptr, "pause");
    ResourceManager::LoadShader("resources/shaders/box.vert", "resources/shaders/box.frag", nullptr, "box");


	GUI = new NuklearRenderer(glfwGetCurrentContext()); // or pass `window` if you store it


    // Global TextRenderer
    auto text = ResourceManager::LoadTextRenderer("default", Width, Height);
    text->Load("resources/fonts/OCRAEXT.TTF", 20); // or your font
    auto textPause = ResourceManager::LoadTextRenderer("pause", Width, Height);
    textPause->Load("resources/fonts/OCRAEXT.TTF", 25); // or your font
    // Load level 0
    levelManager_.LoadLevel(0, Width, Height);
    pauseMenu.SetLevels({ "Level 1", "Level 2", "Level 3" });
}

void Game::Update(float dt)
{
	levelManager_.Update(dt);
}
void Game::ProcessInput(GLFWwindow* window, float dt)
{
    static bool pausePressed = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !pausePressed) {
        isPaused = !isPaused;
        pauseMenu.SetActive(isPaused);
        pausePressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        pausePressed = false;
    }

    if (isPaused) {
        return;
    }

    levelManager_.ProcessInput(dt, Keys);
}





void Game::Render()
{

	const glm::mat4& projection = levelManager_.GetCurrentLevel()->GetProjection();
	levelManager_.Render(projection);

}

void Game::SetSize(unsigned int width, unsigned int height)
{
	this->Width = width;
	this->Height = height;
}

void Game::HandlePauseMenuSelection(PauseMenu::Option opt, GLFWwindow* window) {
	switch (opt) {
		case PauseMenu::RESUME:
			isPaused = false;
			pauseMenu.SetActive(false);
			break;
		case PauseMenu::CHANGE_LEVEL:
			isPaused = true;

			break;
		case PauseMenu::QUIT:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
	}
}
void Game::RenderUI() {
	if (!GUI) return;

	struct nk_context* ctx = GUI->GetContext();
	if (!pauseMenu.IsActive()) return;

	// Get current window size (so Nuklear scales correctly)
	glfwGetFramebufferSize(glfwGetCurrentContext(), (int*)&Width, (int*)&Height);

	// Dynamic size: 30% width, 40% height
	float menuWidth = Width * 0.3f;
	float menuHeight = Height * 0.4f;

	// Centered position
	float menuX = (Width - menuWidth) / 2.0f;
	float menuY = (Height - menuHeight) / 2.0f;

	// Level selector content
	static int selectedLevel = 0;
	static const char* levels[] = { "Level 1", "Level 2", "Level 3" };
	static int numLevels = sizeof(levels) / sizeof(levels[0]);



nk_style backup = ctx->style;

	// Apply styling (colors and padding)
	nk_style *style = &ctx->style;

	// Set window background and border
	style->window.fixed_background = nk_style_item_color(nk_rgba(20, 20, 20, 220)); // dark semi-transparent
	style->window.border_color = nk_rgb(80, 80, 80);
	style->window.rounding = 10;
	style->window.border = 2.0f;
	style->window.padding = nk_vec2(15, 15);

	// Set button appearance
	style->button.normal = nk_style_item_color(nk_rgb(50, 50, 50));
	style->button.hover = nk_style_item_color(nk_rgb(70, 70, 70));
	style->button.active = nk_style_item_color(nk_rgb(90, 90, 90));
	style->button.border_color = nk_rgb(120, 120, 120);
	style->button.text_background = nk_rgb(0, 0, 0);
	style->button.text_normal = nk_rgb(230, 230, 230);
	style->button.text_hover = nk_rgb(255, 255, 255);
	style->button.text_active = nk_rgb(255, 255, 200);

	style->button.border = 1.0f;
	style->button.rounding = 6;
	style->button.padding = nk_vec2(8, 4);

	// Label font color
	style->text.color = nk_rgb(240, 240, 240);


	if (nk_begin(ctx, "Pause Menu", nk_rect(menuX, menuY, menuWidth, menuHeight),
	             NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
		nk_layout_row_dynamic(ctx, 35, 1);
		nk_label(ctx, "Game Paused", NK_TEXT_CENTERED);

		nk_spacing(ctx, 1);

		if (nk_button_label(ctx, "▶ Resume")) {
			isPaused = false;
			pauseMenu.SetActive(false);
		}

		nk_spacing(ctx, 1);

		nk_label(ctx, "Change Level:", NK_TEXT_LEFT);
		if (nk_combo_begin_label(ctx, levels[selectedLevel], nk_vec2(menuWidth - 40.0f, 150))) {
			nk_layout_row_dynamic(ctx, 25, 1);
			for (int i = 0; i < numLevels; ++i) {
				if (nk_combo_item_label(ctx, levels[i], NK_TEXT_LEFT)) {
					selectedLevel = i;
					levelManager_.LoadLevel(i, Width, Height);
					isPaused = false;
					pauseMenu.SetActive(false);
				}
			}
			nk_combo_end(ctx);
		}

		nk_spacing(ctx, 1);

		if (nk_button_label(ctx, "⏻ Quit")) {
			glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
		}
	}
	nk_end(ctx);
	ctx->style = backup;
}

void Game::SetUIRenderer(NuklearRenderer *gui) {
	GUI = gui;
}
