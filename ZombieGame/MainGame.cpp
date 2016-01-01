#include "MainGame.h"

#include <GameEngine/GameEngine.h>
#include <GameEngine\Timing.h>
#include <GameEngine\Errors.h>

#include <SDL/SDL.h>
#include <iostream>

#include <random>
#include <ctime>

#include <algorithm>

#include "Zombie.h"

#include "Gun.h"




const float PLAYER_SPEED = 5.0f;
const float HUMAN_SPEED = 1.0f;
const float ZOMBIE_SPEED = 1.3f;


MainGame::MainGame() :
_gameState(GameState::PLAY),
_fps(0),
_player(nullptr),
_numHumansKilled(0),
_numZombiesKilled(0){
	// Empty
}

MainGame::~MainGame() {
	//Delete levels, humans and zombies
	for (int i = 0; i < _levels.size(); i++)
	{
		delete _levels[i];
	}
	for (int i = 0; i < _humans.size(); i++)
	{
		delete _humans[i];
	}
	for (int i = 0; i < _zombies.size(); i++)
	{
		delete _zombies[i];
	}
}

void MainGame::run() {

	initSystems();

	initLevel();

	gameLoop();
}

void MainGame::initSystems() {
	GameEngine::init();

	_window.create("Game Engine", _screenWidth, _screenHeight, 0);
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

	initShaders();

	_agentSpriteBatch.init();
	_hudSpriteBatch.init();

	//Initialize sprite font
	_spriteFont = new GameEngine::SpriteFont("Fonts/chintzy_cpu_brk/chintzy.ttf", 64);

	//Set up the cameras
	_camera.init(_screenWidth, _screenHeight);
	_hudCamera.init(_screenWidth, _screenHeight);
	_hudCamera.setPosition(glm::vec2(_screenWidth / 2, _screenHeight / 2));
}


void MainGame::initLevel(){
	// Level 1
	_levels.push_back(new Level("Levels/level2.txt"));
	_currentLevel = 0;

	_player = new Player();
	_player->init(PLAYER_SPEED, _levels[_currentLevel]->getStartPlayerPos(), &_inputManager, &_camera, &_bullets);

	_humans.push_back(_player);

	std::mt19937 randomEngine;
	randomEngine.seed(time(nullptr));
	std::uniform_int_distribution<int> randX(2, _levels[_currentLevel]->getWidth() - 2);
	std::uniform_int_distribution<int> randY(2, _levels[_currentLevel]->getHeight() - 2);

	//Add all the random humans
	for (int i = 0; i < _levels[_currentLevel]->getNumHumans(); i++)
	{
		_humans.push_back(new Human);
		glm::vec2 pos(randX(randomEngine) * TILE_WIDTH, randY(randomEngine) * TILE_WIDTH);
		_humans.back()->init(HUMAN_SPEED, pos);
	}

	//Add all the zombies
	const std::vector<glm::vec2> zombiePositions = _levels[_currentLevel]->getStartZombiePositions();
	for (int i = 0; i < zombiePositions.size(); i++)
	{
		_zombies.push_back(new Zombie);
		_zombies.back()->init(ZOMBIE_SPEED, zombiePositions[i]);
	}

	//Set up the guns of the player (pistol, shot gun, machine gun)
	const float BULLET_SPEED = 20.0f;
	_player->addGun(new Gun("Magnum", 10, 1, 5.0f, BULLET_SPEED, 30.0f));
	_player->addGun(new Gun("Shotgun", 30, 20, 20.0f, BULLET_SPEED, 4.0f));
	_player->addGun(new Gun("MP5", 2, 1, 12.0f, BULLET_SPEED, 20.0f));
}

void MainGame::initShaders() {
	// Compile our color shader
	_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
	_textureProgram.addAttribute("vertexPosition");
	_textureProgram.addAttribute("vertexColor");
	_textureProgram.addAttribute("vertexUV");
	_textureProgram.linkShaders();
}

void MainGame::gameLoop() {

	const float DESIRED_FPS = 60.0f;
	const int MAX_PHYSICS_STEPS = 6;

	GameEngine::FpsLimiter fpsLimiter;
	fpsLimiter.setMaxFPS(600000.0f);

	const float CAMERA_SCALE = 1.0f / 4.0f;
	_camera.setScale(CAMERA_SCALE);

	const float MS_PER_SECOND = 1000;
	const float DESIRED_FRAMETIME = MS_PER_SECOND / DESIRED_FPS;
	const float MAX_DELTA_TIME = 1.0f;

	float previousTicks = SDL_GetTicks();


	while (_gameState == GameState::PLAY)
	{
		float newTicks = SDL_GetTicks();
		float frameTime = newTicks - previousTicks;
		previousTicks = newTicks;
		float totalDeltaTime = frameTime / DESIRED_FRAMETIME;

		fpsLimiter.beginFrame();

		checkVictory();

		_inputManager.update();

		processInput();

		int i = 0;
		while (totalDeltaTime > 0.0f && i < MAX_PHYSICS_STEPS)
		{
			float deltaTime = std::min(totalDeltaTime, MAX_DELTA_TIME);
			updateAgents(deltaTime);

			updateBullets(deltaTime);

			totalDeltaTime -= deltaTime;
			i++;
		}

		_camera.setPosition(_player->getPosition());

		_camera.update();

		_hudCamera.update();

		drawGame();

		_fps = fpsLimiter.end();
		std::cout << _fps << std::endl;
	}
}

void MainGame::updateAgents(float deltaTime){
	//Update all humans
	for (int i = 0; i < _humans.size(); i++)
	{
		_humans[i]->update(_levels[_currentLevel]->getLevelData(), _humans, _zombies, deltaTime);
	}

	//Update all zombies
	for (int i = 0; i < _zombies.size(); i++)
	{
		_zombies[i]->update(_levels[_currentLevel]->getLevelData(), _humans, _zombies, deltaTime);
	}

	//Update zombie collisions
	for (int i = 0; i < _zombies.size(); i++)
	{
		//Collide with other zombies
		for (int j = i + 1; j < _zombies.size(); j++) {
			_zombies[i]->collideWithAgent(_zombies[j]);
		}

		//Collide with humans (player is zero so skip him)
		for (int j = 1; j < _humans.size(); j++) {
			if (_zombies[i]->collideWithAgent(_humans[j])){
				//Add the new zombie
				_zombies.push_back(new Zombie);
				_zombies.back()->init(ZOMBIE_SPEED, _humans[j]->getPosition());
				//Delete the human
				delete _humans[j];
				_humans[j] = _humans.back();
				_humans.pop_back();
			}
		}

		//Collide with player
		if (_zombies[i]->collideWithAgent(_humans[0])){
			std::printf("Killed by zombie %d with health: %d at Pos: %d, %d \n", i, _zombies[i]->getHealth(), _zombies[i]->getPosition().x, _zombies[i]->getPosition().y);
			GameEngine::fatalError("YOU LOSE");
		}
	}

	//Update human collisions
	for (int i = 0; i < _humans.size(); i++)
	{
		//Collide with other humans
		for (int j = i + 1; j < _humans.size(); j++) {
			_humans[i]->collideWithAgent(_humans[j]);
		}
	}
}

void MainGame::updateBullets(float deltaTime){
	//Update and collide with world
	for (int i = 0; i < _bullets.size();)
	{
		//if update returns true, the bullet collided with a wall
		if (_bullets[i].update(_levels[_currentLevel]->getLevelData(), deltaTime)){
			_bullets[i] = _bullets.back();
			_bullets.pop_back();
		}
		else{
			i++;
		}
	}

	bool wasBulletRemoved;

	//Collide with agents (humans and zombies)
	for (int i = 0; i < _bullets.size(); i++)
	{
		wasBulletRemoved = false;
		//Loop through zombies
		for (int j = 0; j < _zombies.size(); j++)
		{
			if (_bullets[i].collideWithAgent(_zombies[j])){
				//Damage zombie and kill it if its out of health
				if (_zombies[j]->applyDamage(_bullets[i].getDamage())){
					//If the zombie died, remove it
					delete _zombies[j];
					_zombies[j] = _zombies.back();
					_zombies.pop_back();
					_numZombiesKilled++;
				}
				else{
					j++;
				}

				//Remove the bullet 
				_bullets[i] = _bullets.back();
				_bullets.pop_back();

				wasBulletRemoved = true;

				i--; //Make sure we don't skip a bullet

				//Since the bullet died, no need to loop though the rest of the agents
				break;
			}
			else{
				j++;
			}
		}

		//Loop through the humans
		if (wasBulletRemoved == false)
		{
			for (int j = 1; j < _humans.size(); j++)
			{
				if (_bullets[i].collideWithAgent(_humans[j])){
					//Damage human and kill it if its out of health
					if (_humans[j]->applyDamage(_bullets[i].getDamage())){
						//If the human died, remove it
						delete _humans[j];
						_humans[j] = _humans.back();
						_humans.pop_back();
						_numHumansKilled++;
					}
					else{
						j++;
					}

					//Remove the bullet 
					_bullets[i] = _bullets.back();
					_bullets.pop_back();

					wasBulletRemoved = true;

					i--; //Make sure we don't skip a bullet

					//Since the bullet died, no need to loop though the rest of the agents
					break;
				}
				else{
					j++;
				}
			}
		}

	}
}

void MainGame::checkVictory(){
	//TODO: Support for multiple levels!
	// _currentLevel++; initLevel(...);
	//If all zombies are dead we win!
	if (_zombies.empty()){

		std::printf("*** YOU WIN! ***\n You killed %d humans and %d zombies. There are %d out of %d civilians remaining.", _numHumansKilled, _numZombiesKilled, _humans.size() - 1, _levels[_currentLevel]->getNumHumans());

		GameEngine::fatalError("");
	}
}

void MainGame::processInput() {
	SDL_Event evnt;
	//Will keep looping until there are no more events to process
	while (SDL_PollEvent(&evnt)) {
		switch (evnt.type) {
		case SDL_QUIT:
			// Exit the game here!
			_gameState = GameState::EXIT;
			break;
		case SDL_MOUSEMOTION:
			_inputManager.setMouseCoords(evnt.motion.x, evnt.motion.y);
			break;
		case SDL_KEYDOWN:
			_inputManager.pressKey(evnt.key.keysym.sym);
			break;
		case SDL_KEYUP:
			_inputManager.releaseKey(evnt.key.keysym.sym);
			break;
		case SDL_MOUSEBUTTONDOWN:
			_inputManager.pressKey(evnt.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			_inputManager.releaseKey(evnt.button.button);
			break;
		}
	}
}

void MainGame::drawGame() {
	// Set the base depth to 1.0
	glClearDepth(1.0);
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_textureProgram.use();

	//Draw code goes here
	glActiveTexture(GL_TEXTURE0);

	//Make sure the shader uses texture 0
	GLint textureUniform = _textureProgram.getUniformLocation("mySampler");
	glUniform1i(textureUniform, 0);

	//Grab the camera matrix
	glm::mat4 projectionMatrix = _camera.getCameraMatrix();
	GLint pUniform = _textureProgram.getUniformLocation("P");
	glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

	//Draw the level
	_levels[_currentLevel]->draw();

	//Begin drawing agents
	_agentSpriteBatch.begin();

	const glm::vec2 agentDims(AGENT_RADIUS * 2.0f);

	//Draw the humans
	for (int i = 0; i < _humans.size(); i++)
	{
		if (_camera.isBoxInView(_humans[i]->getPosition(), agentDims))
		{
			_humans[i]->draw(_agentSpriteBatch);
		}
	}

	//Draw the zombies
	for (int i = 0; i < _zombies.size(); i++)
	{
		if (_camera.isBoxInView(_zombies[i]->getPosition(), agentDims))
		{
			_zombies[i]->draw(_agentSpriteBatch);
		}
	}

	//Draw the bullets
	for (int i = 0; i < _bullets.size(); i++)
	{
		_bullets[i].draw(_agentSpriteBatch);
	}

	_agentSpriteBatch.end();

	_agentSpriteBatch.renderBatch();

	//Render the heads up display
	drawHUD();

	_textureProgram.unuse();

	// Swap our buffer and draw everything to the screen!
	_window.swapBuffer();
}

void MainGame::drawHUD(){
	char buffer[256];

	//Grab the hud camera matrix
	glm::mat4 projectionMatrix = _hudCamera.getCameraMatrix();
	GLint pUniform = _textureProgram.getUniformLocation("P");
	glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

	_hudSpriteBatch.begin();

	sprintf_s(buffer, "Num Humans %d", _humans.size());
	_spriteFont->draw(_hudSpriteBatch, buffer, glm::vec2(0, 0), glm::vec2(0.5), 0.0f, GameEngine::ColorRGBA8(255, 255, 255, 255));

	sprintf_s(buffer, "Num Zombies %d", _zombies.size());
	_spriteFont->draw(_hudSpriteBatch, buffer, glm::vec2(0, 36), glm::vec2(0.5), 0.0f, GameEngine::ColorRGBA8(255, 255, 255, 255));

	_hudSpriteBatch.end();

	_hudSpriteBatch.renderBatch();
}