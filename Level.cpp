// Level.cpp
#include "Level.h"

#include <GLFW/glfw3.h>

#include "Dog.h"
#include "RESOURCE_MANAGER.h"
#include "Enemies.h"


Level::Level(): dog_(nullptr) {
}

Level::~Level() { Unload(); }

void Level::Unload() {
	tileMap.reset();
	enemies.clear();
	solidTiles.clear();
}

void Level::Load(int index, unsigned int width, unsigned int height) {

	Unload(); // clean up previous level
	projection_ = glm::ortho(0.0f, INTERNAL_WIDTH, INTERNAL_HEIGHT, 0.0f);

	// Load shader and texture assets
	ResourceManager::LoadShader("resources/shaders/sprite.vert", "resources/shaders/sprite.frag", nullptr, "sprite");
	ResourceManager::LoadTexture("resources/textures/48DogSpriteSheet.png", true, "dog");
	ResourceManager::LoadTexture("resources/textures/Slime.png", true, "slime");
	ResourceManager::LoadTexture("resources/textures/Skeleton.png", true, "skeleton");
	ResourceManager::LoadTexture("resources/textures/DesertTilemap16x16.png", true, "TileMap");
	ResourceManager::LoadShader("resources/shaders/line.vert", "resources/shaders/line.frag", nullptr, "grid");
	Shader shader = ResourceManager::GetShader("sprite");
	Texture2D tileTex = ResourceManager::GetTexture("TileMap");

	tileMap = std::make_unique<TileMap>(shader, tileTex, 224, 240, 16, 16);
	// Tile map
TextRenderer* textRenderer = new TextRenderer(width, height);
	textRenderer->Load("resources/fonts/OCRAEXT.TTF", 15);  // Or any valid .ttf

	// Pass the renderer to TileMap
	tileMap->SetTextRenderer(textRenderer);
	// Set your level layout data per index (use JSON or hardcoded)
	if (index == 0) {
		tileMap->Load({
    {  45,  13,  13,  14,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  14,  13,  13,  13,  13,  13,  13,  13,  45 },
    {  13,  13,  13,  14,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  13,  13,  13,  13,  14,  13,  13,  27,  27,  27,  27,  27,  27 },
    {  13,  13,  13,  14,  13,  13,  13,  13,  13,  13,  13,  13, 126, 127, 128, 129,  13,  13,  27,  27,  27,  27,  14,  27,  13,  13,  13,  13,  13,  27,  27 },
    {  27,  27,  27,  14,  27,  27,  13,  13,  13,  27,  27,  27, 140, 141, 142, 143,  27,  27,  13,  13,  13,  13,  14,  13,  27,  13,  13,  27,  27,  27,  27 },
    {  13,  13,  13,  14,  13,  13,  27,  27,  27,  13,  13,  27, 154, 155, 156, 157,  27,  13,  13,  13,  13,  13,  14,  13,  13,  13,  13,  27,  13,  27,  27 },
    {  27,  27,  27,  14,  27,  27,  27,  27,  27,  27,  27,  27, 168, 169, 170, 171,  13,  13,  27,  27,  27,  27,  14,  27,  27,  27,  27,  27,  27,  27,  27 },
    {  27,  13,  27,  14,  27,  27,  27,  67,  68,  69,  27,  27, 182, 183, 184, 185,  13,  13,  27,  27,  13,  45,  14,  45,  27,  27,  27,  27,  27,  13,  27 },
    {   1,   1,   1,  24,   1,   1,   1,  81,  82,  83,   1,   1,   1,  21,  21,   1,   1,   1,   1,   1,   1,   1,  24,   1,   1,   1,   1,   1,   1,   1,   1 },
    {  27,  27,  13,  14,  27,  27,  27,  27,  13,  13,  13,  13,  27,  27,  27,  27,  27,  27,  27,  27,  13,  45,  14,  45,  27,  13,  13,  27,  27,  13,  27 },
    {  27,  13,  27,  14,  13,  13,  13,  13,  27,  27,  27,  27,  27,  27,  27,  13,  27,  27,  27,  27,  27,  27,  14,  13,  13,  27,  27,  27,  27,  27,  27 },
    {  27,  13,  13,  14,  13,  27,  27,  27,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  14,  27,  27,  27,  13,  13,  13,  13,  27 },
    {  13,  13,  13,  14,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  14,  27,  27,  27,  27,  27,  27,  27,  27 },
    {  27,  27,  27,  14,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  27,  27,  27,  14,  13,  13,  27,  27,  27,  27,  27,  27 },
    {  27,  27,  27,  14,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  13,  27,  27,  14,  27,  27,  13,  13,  27, 137, 138, 138 },
    {  27,  27,  27,  14,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  14,  27,  27,  27,  27,  27, 151, 152, 152 },
    {  13,  13,  13,  14,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  14,  13,  13,  13,  13,  13, 151, 152, 152 },
    {  45,  27,  27,  14,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  14,  27,  27,  27,  27,  27, 151, 152, 45 },
});

		this->solidTiles
 = {
			126, 127, 128 ,129, 140, 141, 142, 143,
			154, 155, 156, 157, 168 ,169 ,170 ,171,
			182, 183, 184 ,185, 45, 81, 82, 83,
			67, 68, 69, 137, 138, 151, 152
		};





		// Dog
		Texture2D dogTexture = ResourceManager::GetTexture("dog");
		dog_ = new Dog(shader, dogTexture, glm::vec2(50, 50), glm::ivec2(1, 0));
		dog_->SetScale(.5f);

		Texture2D slimeTex = ResourceManager::GetTexture("slime");
		enemies.push_back(std::make_unique<SlimeEnemy>(
			shader, slimeTex, glm::vec2(100, 100),
			glm::ivec2(0, 2), 192.0f, 96.0f, 6, 3));

		Texture2D skeletonTex = ResourceManager::GetTexture("skeleton");
		enemies.push_back(std::make_unique<SkeletonEnemy>(
			shader, skeletonTex, glm::vec2(159, 150),
			glm::ivec2(7, 2), 192.0f, 320.0f, 6, 10));



	}
}

void Level::Update(float dt) {
	for (auto& enemy : enemies)
		enemy->Update(dt, tileMap.get());
dog_->Update(dt, tileMap.get(), solidTiles);
}

void Level::Render(const glm::mat4& proj) {
	if (tileMap) {
		tileMap->Draw(proj);
		tileMap->DrawDebugGrid(proj);
	}
	for (auto& enemy : enemies)
		enemy->Draw(proj);
	dog_->Draw(proj);
}
void Level::ProcessInput(float dt, const bool* keys) {
	glm::vec2 velocity(0.0f);
	if (keys[GLFW_KEY_W]) velocity.y -= 1.0f;
	if (keys[GLFW_KEY_S]) velocity.y += 1.0f;
	if (keys[GLFW_KEY_A]) velocity.x -= 1.0f;
	if (keys[GLFW_KEY_D]) velocity.x += 1.0f;

	if (glm::length(velocity) > 0.0f)
		velocity = glm::normalize(velocity);

	if (dog_)
		dog_->SetVelocity(velocity * 100.0f); // Adjust speed as needed
}
