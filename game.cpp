// ──────────────────────────────────────────────────────────────────────────────
// Game.cpp – polished pause-menu UI + movement / Esc handling
// ──────────────────────────────────────────────────────────────────────────────
#include "game.h"

#include <cstdlib>
#include <ctime>
#include <unordered_set>

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

    auto panelTex  = ResourceManager::LoadTexture(
        "resources/gui/Game Menu/1x/Asset 15 - 1080p.png", "panel9");
    auto buttonTex = ResourceManager::LoadTexture(
        "resources/gui/Game Menu/1x/Asset 8 - 1080p.png",  "button9");
    auto buttonHot = ResourceManager::LoadTexture(
        "resources/gui/Game Menu/1x/Asset 2 - 1080p.png",  "button9_hover");

    panelRenderer    = new NineSliceRenderer(nine);
    btnRenderer      = new NineSliceRenderer(nine);
    btnHoverRenderer = new NineSliceRenderer(nine);

    panelRenderer   ->SetTexture(panelTex,   16);
    btnRenderer     ->SetTexture(buttonTex,  16);
    btnHoverRenderer->SetTexture(buttonHot,  16);

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

// ──────────────────────────────────────────────────────────────────────────────
// RenderUI – unchanged visual logic (fade, panel, buttons, hover, text)
// ─────────────────────────────────────────────────────────────────────────────-
void Game::RenderUI() {
    if (!isPaused) return;

    /* ---------- build pixel-proj, fade quad, 9-slice layout, etc. ------------ */
    int fbW, fbH; glfwGetFramebufferSize(glfwGetCurrentContext(), &fbW, &fbH);
    glm::mat4 uiProj = glm::ortho(0.f,(float)fbW,(float)fbH,0.f,-1.f,1.f);

    /* full-screen fade */
    {
        auto sh = ResourceManager::GetShader("pause");
        sh->Use();
        sh->SetMatrix4("projection", uiProj, false);
        sh->SetVector4f("uColor", {0,0,0,0.55f}, false);
        float v[6][2]={{0,0},{(float)fbW,(float)fbH},{0,(float)fbH},
                       {0,0},{(float)fbW,0},{(float)fbW,(float)fbH}};
        GLuint vao,vbo; glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
        glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*)0);
        glDrawArrays(GL_TRIANGLES,0,6);
        glDeleteBuffers(1,&vbo); glDeleteVertexArrays(1,&vao);
    }

    /* ---- menu geometry ------------------------------------------------------ */
    const float menuW = fbW*0.35f, menuH = fbH*0.45f;
    const float menuX = (fbW-menuW)*0.5f, menuY = (fbH-menuH)*0.5f;
    constexpr float btnH = 90.f, spacing = 18.f;

    /* ---- Nuklear invisible window ------------------------------------------ */
    nk_context* ctx = GUI->GetContext();
    nk_style bak=ctx->style;
    ctx->style.window.background = nk_rgba(0,0,0,0);
    ctx->style.window.border=0; ctx->style.window.padding=nk_vec2(0,0);

    static int levelIdx=0; const char* levels[]={"Level 1","Level 2","Level 3"};
    struct nk_rect titleB{}, resumeB{}, levelB{}, quitB{};
    ctx->style.window.fixed_background = nk_style_item_hide();
    if (nk_begin(ctx,"Pause", nk_rect(menuX,menuY,menuW,menuH),
                 NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_NO_INPUT)){
        nk_layout_row_dynamic(ctx,60,1);
        titleB = nk_widget_bounds(ctx);   nk_label(ctx,"PAUSED", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx,spacing,1); nk_label(ctx,"",NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx,btnH,1);
        resumeB = nk_widget_bounds(ctx);  nk_label(ctx,"RESUME", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx,spacing,1); nk_label(ctx,"",NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx,40,1);  nk_label(ctx,"LEVEL", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx,btnH,1);
        levelB = nk_widget_bounds(ctx);   nk_label(ctx,levels[levelIdx], NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx,spacing,1); nk_label(ctx,"",NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx,btnH,1);
        quitB  = nk_widget_bounds(ctx);   nk_label(ctx,"QUIT", NK_TEXT_CENTERED);
    }
    nk_end(ctx); ctx->style=bak;

    /* ---- button hover tests ------------------------------------------------- */
    double mx,my; glfwGetCursorPos(glfwGetCurrentContext(),&mx,&my);
    auto inside=[&](const struct nk_rect& r){
        return mx>=r.x&&mx<=r.x+r.w&&my>=r.y&&my<=r.y+r.h; };
    bool overRes=inside(resumeB), overLvl=inside(levelB), overQuit=inside(quitB);

    /* ---- draw nine-slice ---------------------------------------------------- */
    NineSliceRenderer::SetDepthTest(false);

    /* panel */
    panelRenderer->Render(menuX, menuY, menuW, menuH,
                          uiProj, glm::vec4(1.0f));

    /* buttons */
    (overRes  ? btnHoverRenderer : btnRenderer)
        ->Render(resumeB.x, resumeB.y, resumeB.w, resumeB.h,
                 uiProj, glm::vec4(1.0f));

    (overLvl  ? btnHoverRenderer : btnRenderer)
        ->Render(levelB.x , levelB.y , levelB.w , levelB.h ,
                 uiProj, glm::vec4(1.0f));

    (overQuit ? btnHoverRenderer : btnRenderer)
        ->Render(quitB.x  , quitB.y  , quitB.w  , quitB.h  ,
                 uiProj, glm::vec4(1.0f));

    NineSliceRenderer::SetDepthTest(true);

    /* ---- text -------------------------------------------------------------- */
    ResourceManager::GetTextRenderer("pause")
        .RenderText("PAUSED", titleB.x+0.5f*(titleB.w-120.f),
                    titleB.y+10.f,1.f,{1,1,1},uiProj);
    auto& s=ResourceManager::GetTextRenderer("default");
    s.RenderText("RESUME", resumeB.x+40.f,resumeB.y+26.f,1.f,{1,1,1},uiProj);
    s.RenderText(levels[levelIdx], levelB.x+40.f,levelB.y+26.f,1.f,{1,1,1},uiProj);
    s.RenderText("QUIT",   quitB.x+40.f ,quitB.y+26.f ,1.f,{1,1,1},uiProj);

    /* ---- simple mouse input ------------------------------------------------- */
    static bool prevLmb=false;
    bool lmb=glfwGetMouseButton(glfwGetCurrentContext(),GLFW_MOUSE_BUTTON_LEFT)
             ==GLFW_PRESS;
    if(lmb&&!prevLmb){
        if(overRes)  isPaused=false;
        else if(overQuit) glfwSetWindowShouldClose(glfwGetCurrentContext(),true);
        else if(overLvl)  levelIdx=(levelIdx+1)%3;
    }
    prevLmb=lmb;

    /* ---- keyboard up/down cycle level list --------------------------------- */
    if(glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_UP)==GLFW_PRESS)   levelIdx=(levelIdx+2)%3;
    if(glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_DOWN)==GLFW_PRESS) levelIdx=(levelIdx+1)%3;
}
