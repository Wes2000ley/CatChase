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
	if (!GUI || !pauseMenu.IsActive()) return;

	struct nk_context* ctx = GUI->GetContext();
	glfwGetFramebufferSize(glfwGetCurrentContext(), (int*)&Width, (int*)&Height);

	// Window sizing
	float menuWidth  = Width * 0.35f;
	float menuHeight = Height * 0.45f;
	float menuX = (Width - menuWidth) / 2.0f;
	float menuY = (Height - menuHeight) / 2.0f;

	// Level selector state
	static int selectedLevel = 0;
	static const char* levels[] = { "Level 1", "Level 2", "Level 3" };
	static const int numLevels = sizeof(levels) / sizeof(levels[0]);

	// Load UI textures
	ResourceManager::LoadTexture("resources/gui/Game Menu/1x/Asset 1 - 1080p.png",true,"panel_bg");
	ResourceManager::LoadTexture("resources/gui/Game Menu/1x/Asset 8 - 1080p.png",  true, "btn_idle");
	ResourceManager::LoadTexture("resources/gui/Game Menu/1x/Asset 2 - 1080p.png", true, "btn_hover");
	ResourceManager::LoadTexture("resources/gui/Game Menu/1x/Asset 2 - 1080p.png", true, "btn_active");


	/*


	// During init:
	auto buttonTex = ResourceManager::LoadTexture("resources/ui_button_9slice.png", true, "button9");
	auto buttonShader = ResourceManager::LoadShader("resources/shaders/9slice.vert", "resources/shaders/9slice.frag", nullptr, "9slice");

	NineSliceRenderer buttonRenderer(buttonShader);
	buttonRenderer.SetTexture(buttonTex, 16);

	// Inside Game::RenderUI()
	buttonRenderer.Render(x, y, w, h);
	*/

	auto panelTex  = ResourceManager::GetTexture("panel_bg");
	auto btnIdle   = ResourceManager::GetTexture("btn_idle");
	auto btnHover  = ResourceManager::GetTexture("btn_hover");
	auto btnActive = ResourceManager::GetTexture("btn_active");

	nk_style backup = ctx->style;

	// === 🪟 Panel background ===
	ctx->style.window.fixed_background = nk_style_item_image(nk_image_id(panelTex->ID));
	ctx->style.window.border = 0;
	ctx->style.window.padding = nk_vec2(20, 20);
	ctx->style.window.rounding = 0;
	ctx->style.window.border_color = nk_rgb(0, 0, 0);

	// === 🟥 Button styling ===
	ctx->style.button.normal  = nk_style_item_image(nk_image_id(btnIdle->ID));
	ctx->style.button.hover   = nk_style_item_image(nk_image_id(btnHover->ID));
	ctx->style.button.active  = nk_style_item_image(nk_image_id(btnActive->ID));
	ctx->style.button.border_color = nk_rgba(0, 0, 0, 0);
	ctx->style.button.text_background = nk_rgba(0, 0, 0, 0);
	ctx->style.button.text_normal = nk_rgb(255, 240, 200);
	ctx->style.button.text_hover = nk_rgb(255, 255, 255);
	ctx->style.button.text_active = nk_rgb(255, 255, 150);
	ctx->style.button.padding = nk_vec2(8, 6);
	ctx->style.button.rounding = 0;


	// === 🔽 Combo/Dropdown styling ===
	ctx->style.combo.normal  = nk_style_item_image(nk_image_id(btnIdle->ID));
	ctx->style.combo.hover   = nk_style_item_image(nk_image_id(btnHover->ID));
	ctx->style.combo.active  = nk_style_item_image(nk_image_id(btnActive->ID));

	ctx->style.combo.border_color = nk_rgba(0, 0, 0, 0);
	ctx->style.combo.label_normal = nk_rgb(255, 240, 200);
	ctx->style.combo.label_hover = nk_rgb(255, 255, 255);
	ctx->style.combo.label_active = nk_rgb(255, 255, 150);

	ctx->style.combo.border = 0.0f;
	ctx->style.combo.rounding = 0;
	ctx->style.combo.content_padding = nk_vec2(8, 6);
	ctx->style.combo.button_padding = nk_vec2(4, 4);
	ctx->style.combo.spacing = nk_vec2(4, 4);


if (nk_begin(ctx, "", nk_rect(menuX, menuY, menuWidth, menuHeight),
             NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NO_INPUT)) {
	// Resume button
	nk_layout_row_dynamic(ctx, 128, 1);
	if (nk_button_label(ctx, "▶ Resume")) {
		isPaused = false;
		pauseMenu.SetActive(false);
	}

	// Small spacer
	nk_layout_row_static(ctx, 12, 1, 1); // 12px vertical gap

	// Label + dropdown
	nk_layout_row_dynamic(ctx, 128, 1);
	nk_label(ctx, "Change Level:", NK_TEXT_LEFT);

	nk_layout_row_dynamic(ctx, 128, 1); // Combo box as tall as buttons
	if (nk_combo_begin_label(ctx, levels[selectedLevel], nk_vec2(menuWidth - 40.0f, 100.0f))) {
		nk_layout_row_dynamic(ctx, 32, 1); // height of combo items
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

	// Small spacer
	nk_layout_row_static(ctx, 12, 1, 1);

	// Quit button
	nk_layout_row_dynamic(ctx, 128, 1);
	if (nk_button_label(ctx, "⏻ Quit Game")) {
		glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
	}

	}
	nk_end(ctx);

	ctx->style = backup;
}


void Game::SetUIRenderer(NuklearRenderer *gui) {
	GUI = gui;
}
