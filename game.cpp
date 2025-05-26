// ──────────────────────────────────────────────────────────────────────────────
// Game.cpp – polished pause-menu UI + movement / Esc handling
// ──────────────────────────────────────────────────────────────────────────────
#include "game.h"

#include <cstdlib>
#include <ctime>
#include <unordered_set>
#include "TEXT_RENDERER.h"

#include "Dog.h"
#include "Enemies.h"
#include "Enemy.h"
#include "RESOURCE_MANAGER.h"
#include "TileMap.h"
#include "Collision.h"
#include "LevelManager.h"
#include "NineSliceRenderer.h"
#include "TEXT_RENDERER.h"
#include "PauseMenu.h"
#include "utility.h"

#include <GLFW/glfw3.h>
#include "nuklear.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



using std::string;
using std::vector;

PauseMenu pauseMenu;
bool       isPaused = false;

// ──────────────────────────────────────────────────────────────────────────────
Game::Game(unsigned int w, unsigned int h)
    : State(GAME_ACTIVE), Width(w), Height(h)
{
    std::fill(std::begin(Keys), std::end(Keys), false);
}
Game::~Game() { ResourceManager::Clear(); }

// ──────────────────────────────────────────────────────────────────────────────
void Game::Init() {

    srand(static_cast<unsigned>(time(nullptr)));

    ResourceManager::LoadShader("resources/shaders/pause.vert",
                                "resources/shaders/pause.frag", nullptr, "pause");

    auto nine = ResourceManager::LoadShader("resources/shaders/9slice.vert",
                                            "resources/shaders/9slice.frag", nullptr, "9slice");


    auto panelTex = ResourceManager::LoadTexture(
        "resources/gui/Game Menu/pausePanel.png", "pausepanel");
    auto continueText = ResourceManager::LoadTexture(
        "resources/gui/Game Menu/Continue.png", "Continue");
    auto continueHotText = ResourceManager::LoadTexture(
        "resources/gui/Game Menu/Continue-h.png", "ContinueH");
    auto levelSelectTex = ResourceManager::LoadTexture(
        "resources/gui/Game Menu/LevelSelect.png", "Select");
    auto levelSelectHotTex = ResourceManager::LoadTexture(
        "resources/gui/Game Menu/LevelSelect-h.png", "SelectH");
    auto quitTex = ResourceManager::LoadTexture(
        "resources/gui/Game Menu/quitegame.png", "Quite");
    auto quitHotTex = ResourceManager::LoadTexture(
        "resources/gui/Game Menu/quitgame-h.png", "QuiteH");

    panelRenderer = new NineSliceRenderer(nine);
    continueRenderer = new NineSliceRenderer(nine);
    continuehotRenderer = new NineSliceRenderer(nine);
    levelSelectRenderer = new NineSliceRenderer(nine);
    levelSelectHotRenderer = new NineSliceRenderer(nine);
    quitRenderer = new NineSliceRenderer(nine);
    quitHotRenderer = new NineSliceRenderer(nine);


    panelRenderer->SetTexture(panelTex, 16);
    continueRenderer->SetTexture(continueText, 16);
    continuehotRenderer->SetTexture(continueHotText, 16);
    levelSelectRenderer->SetTexture(levelSelectTex, 16);
    levelSelectHotRenderer->SetTexture(levelSelectHotTex, 16);
    quitRenderer->SetTexture(quitTex, 16);
    quitHotRenderer->SetTexture(quitHotTex, 16);


    ResourceManager::LoadTextRenderer("default", Width, Height)
            ->Load("resources/fonts/OCRAEXT.TTF", 20);
    ResourceManager::LoadTextRenderer("pause", Width, Height)
        ->Load("resources/fonts/OCRAEXT.TTF", 28);

    levelManager_.LoadLevel(0, Width, Height);
    pauseMenu.SetLevels({ "Level 1", "Level 2", "Level 3" });

    GUI = new NuklearRenderer(glfwGetCurrentContext());
}

// ──────────────────────────────────────────────────────────────────────────────
void Game::Update(float dt) {
    if (!isPaused) levelManager_.Update(dt);
}

/* ---------------------------------------------------------------------------
 *  ProcessInput – updates Keys[ ] every frame, toggles pause on Esc,
 *                 and forwards controls to Level when not paused.
 * ------------------------------------------------------------------------- */
void Game::ProcessInput(GLFWwindow* window, float dt) {

    /* --- toggle pause with Esc ------------------------------------------------ */
    static bool escHeld = false;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escHeld) {
        isPaused = !isPaused;
        escHeld  = true;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) escHeld = false;

    /* --- fill key array (0-1023) --------------------------------------------- */
    for (int i = 0; i < 1024; ++i)
        Keys[i] = glfwGetKey(window, i) == GLFW_PRESS;

    /* --- send WASD etc. to current level when not paused --------------------- */
    if (!isPaused)
        levelManager_.ProcessInput(dt, Keys);
}

// ──────────────────────────────────────────────────────────────────────────────
void Game::Render() {
    levelManager_.Render(levelManager_.GetCurrentLevel()->GetProjection());
}
void Game::SetSize(unsigned w, unsigned h) { Width = w; Height = h; }
void Game::SetUIRenderer(NuklearRenderer* g) { GUI = g; }
void Game::HandlePauseMenuSelection(PauseMenu::Option, GLFWwindow*) {}


void Game::RenderUI() {
    if (!isPaused) return;

    // 0) Setup pixel space projection
    int fbW, fbH;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &fbW, &fbH);
    glm::mat4 uiProj = glm::ortho(0.f, (float)fbW, (float)fbH, 0.f, -1.f, 1.f);

    // 1) Fullscreen fade
    {
        auto sh = ResourceManager::GetShader("pause");
        sh->Use();
        sh->SetMatrix4("projection", uiProj, false);
        sh->SetVector4f("uColor", {0, 0, 0, 0.55f}, false);
        float quad[6][2] = {
            {0,0}, { (float)fbW,(float)fbH }, { 0,(float)fbH },
            {0,0}, { (float)fbW,0 },           { (float)fbW,(float)fbH }
        };
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    // 2) Menu panel rectangle (percent of screen)
constexpr float MENU_X_PCT = 0.35f;
    constexpr float MENU_Y_PCT = 0.25f;
    constexpr float MENU_W_PCT = 0.30f;
    constexpr float MENU_H_PCT = 0.50f;
    constexpr float SMALL_FONT_PX = 20.f;


    float menuX = MENU_X_PCT * fbW;
    float menuY = MENU_Y_PCT * fbH;
    float menuW = MENU_W_PCT * fbW;
    float menuH = MENU_H_PCT * fbH;

    // 3) Element rectangles **inside** the menu, also pct of menu
    //    Tweak these to reposition/resize each element:
    struct ElemDef {
        float xPct, yPct, wPct, hPct;
        NineSliceRenderer* normal, *hover;
    };
const ElemDef elems[] = {
        // continue button
        {0.20f, 0.22f, 0.60f, 0.133f, continueRenderer, continuehotRenderer},
        // level-select button
        {0.20f, 0.44f, 0.60f, 0.133f, levelSelectRenderer, levelSelectHotRenderer},
        // quit button
        {0.20f, 0.66f, 0.60f, 0.133f, quitRenderer, quitHotRenderer}
    };
    // 4) Draw panel
    NineSliceRenderer::SetDepthTest(false);
    panelRenderer->Render(menuX, menuY, menuW, menuH, uiProj);

    // 5) Cursor for hover
    double cx, cy;
    glfwGetCursorPos(glfwGetCurrentContext(), &cx, &cy);

    // 6) Iterate elements
    for (int i = 0; i < 3; ++i) {
        auto &d = elems[i];
        // pixel rect:
        float ex = menuX + d.xPct * menuW;
        float ey = menuY + d.yPct * menuH;
        float ew = d.wPct * menuW;
        float eh = d.hPct * menuH;

        bool hover = (cx >= ex && cx <= ex + ew && cy >= ey && cy <= ey + eh);
        (hover ? d.hover : d.normal)
            ->Render(ex, ey, ew, eh, uiProj);
    }

    NineSliceRenderer::SetDepthTest(true);

    // 7) Draw text centered in each
    auto &big = ResourceManager::GetTextRenderer("pause");
    auto &small = ResourceManager::GetTextRenderer("default");
    float txtScale = 1.0f;  // or compute font-scale as you like

    // “PAUSED” title at top of panel
    {
        float tw = big.MeasureTextWidth("PAUSED", txtScale);
        float tx = menuX + 0.5f * (menuW - tw);
        float ty = menuY + 0.05f * menuH;           // 5% down from panel top
        big.RenderText("PAUSED", tx, ty, txtScale, {1,1,1}, uiProj);
    }

    // button labels:
    constexpr const char* btnLabels[] = { "CONTINUE", "LEVEL SELECT", "QUIT" };
    for (int i = 0; i < 3; ++i) {
        auto &d = elems[i];
        float ex = menuX + d.xPct * menuW;
        float ey = menuY + d.yPct * menuH;
        float ew = d.wPct * menuW;
        float eh = d.hPct * menuH;

        // measure text width
        const char* txt = btnLabels[i];
        float tw = small.MeasureTextWidth(txt, txtScale);
        // compute height of text in pixels
        float th = SMALL_FONT_PX * txtScale;

        // center x, y
        float tx = ex + 0.5f * (ew - tw);
        float ty = ey + 0.5f * (eh - th);
        small.RenderText(btnLabels[i], tx, ty, txtScale, {1,1,1}, uiProj);
    }

    // 8) Simple input
    static bool prevL = false;
    bool l = glfwGetMouseButton(glfwGetCurrentContext(),
                                GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (l && !prevL) {
        if      (cx >= menuX + elems[0].xPct*menuW && cx <= menuX + (elems[0].xPct+elems[0].wPct)*menuW
              && cy >= menuY + elems[0].yPct*menuH && cy <= menuY + (elems[0].yPct+elems[0].hPct)*menuH)
            isPaused = false;
        else if (cx >= menuX + elems[2].xPct*menuW && cx <= menuX + (elems[2].xPct+elems[2].wPct)*menuW
              && cy >= menuY + elems[2].yPct*menuH && cy <= menuY + (elems[2].yPct+elems[2].hPct)*menuH)
            glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
        else if (cx >= menuX + elems[1].xPct*menuW && cx <= menuX + (elems[1].xPct+elems[1].wPct)*menuW
              && cy >= menuY + elems[1].yPct*menuH && cy <= menuY + (elems[1].yPct+elems[1].hPct)*menuH)
            /* your level-cycle logic here */;
    }
    prevL = l;
}
