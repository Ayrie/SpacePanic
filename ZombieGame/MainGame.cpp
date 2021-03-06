#include "MainGame.h"

#include <GameEngine/GameEngine.h>
#include <GameEngine\Timing.h>
#include <GameEngine\GameEngineErrors.h>
#include <GameEngine\ResourceManager.h>

#include <SDL/SDL.h>
#include <iostream>

#include <random>
#include <ctime>
#include <glm\gtx\rotate_vector.hpp>

#include <algorithm>

#include "Zombie.h"

#include "Gun.h"


const float DEG_TO_RAD = M_PI / 180.0f;
const float RAD_TO_DEG = 180.0f / M_PI;


const float PLAYER_SPEED = 5.0f;
const float HUMAN_SPEED = 1.0f;
const float ZOMBIE_SPEED = 1.3f;


MainGame::MainGame() :
m_gameState(GameState::PLAY),
m_fps(0),
m_player(nullptr),
m_numHumansKilled(0),
m_numZombiesKilled(0){
	// Empty
}

MainGame::~MainGame() {
	//Delete levels, humans and zombies
	for (int i = 0; i < m_levels.size(); i++)
	{
		delete m_levels[i];
	}
	for (int i = 0; i < m_humans.size(); i++)
	{
		delete m_humans[i];
	}
	for (int i = 0; i < m_zombies.size(); i++)
	{
		delete m_zombies[i];
	}
}

void MainGame::run() {

	initSystems();

	initLevel();

	//Start playing music
	GameEngine::Music music = m_audioEngine.loadMusic("Sound/XYZ.ogg");
	music.play(-1);

	gameLoop();
}

void MainGame::initSystems() {
	//Initialize the game engine
	GameEngine::init();

	//Initialize sound, must happen after GameEngine::init()
	m_audioEngine.init();

	m_window.create("Game Engine", m_screenWidth, m_screenHeight, 0);
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

	initShaders();

	m_agentSpriteBatch.init();
	m_hudSpriteBatch.init();

	//Initialize sprite font
	m_spriteFont = new GameEngine::SpriteFont("Fonts/chintzy_cpu_brk/chintzy.ttf", 64);

	//Set up the cameras
	m_camera.init(m_screenWidth, m_screenHeight);
	m_hudCamera.init(m_screenWidth, m_screenHeight);
	m_hudCamera.setPosition(glm::vec2(m_screenWidth / 2, m_screenHeight / 2));

	//Initialize particles
	m_bloodParticleBatch = new GameEngine::ParticleBatch2D();

	m_bloodParticleBatch->init(
		1000, 0.05f, 
		GameEngine::ResourceManager::getTexture("Textures/particle.png"), 
		[](GameEngine::Particle2D& particle, float deltaTime) {
		particle.position += particle.velocity * deltaTime;
		particle.color.a = (GLubyte)(particle.life * 255.0f); 
	}); // using a Lambda to create a function to give as a function pointer

	m_particleEngine.addParticleBatch(m_bloodParticleBatch);
}


void MainGame::initLevel(){
	// Level 1
	m_levels.push_back(new Level("Levels/level2.txt"));
	m_currentLevel = 0;

	m_player = new Player();
	m_player->init(PLAYER_SPEED, m_levels[m_currentLevel]->getStartPlayerPos(), &m_inputManager, &m_camera, &m_bullets);

	m_humans.push_back(m_player);

	std::mt19937 randomEngine;
	randomEngine.seed(time(nullptr));
	std::uniform_int_distribution<int> randX(2, m_levels[m_currentLevel]->getWidth() - 2);
	std::uniform_int_distribution<int> randY(2, m_levels[m_currentLevel]->getHeight() - 2);

	//Add all the random humans
	for (int i = 0; i < m_levels[m_currentLevel]->getNumHumans(); i++)
	{
		m_humans.push_back(new Human);
		glm::vec2 pos(randX(randomEngine) * TILE_WIDTH, randY(randomEngine) * TILE_WIDTH);
		m_humans.back()->init(HUMAN_SPEED, pos);
	}

	//Add all the zombies
	const std::vector<glm::vec2> zombiePositions = m_levels[m_currentLevel]->getStartZombiePositions();
	for (int i = 0; i < zombiePositions.size(); i++)
	{
		m_zombies.push_back(new Zombie);
		m_zombies.back()->init(ZOMBIE_SPEED, zombiePositions[i]);
	}

	//Set up the guns of the player (pistol, shot gun, machine gun)
	const float BULLET_SPEED = 20.0f;
	m_player->addGun(new Gun("Magnum", 10, 1, 5.0f, BULLET_SPEED, 30.0f, m_audioEngine.loadSoundEffect("Sound/shots/pistol.wav")));
	m_player->addGun(new Gun("Shotgun", 30, 20, 20.0f, BULLET_SPEED, 4.0f, m_audioEngine.loadSoundEffect("Sound/shots/shotgun.wav")));
	m_player->addGun(new Gun("MP5", 2, 1, 12.0f, BULLET_SPEED, 20.0f, m_audioEngine.loadSoundEffect("Sound/shots/cg1.wav")));
}

void MainGame::initShaders() {
	// Compile our color shader
	m_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
	m_textureProgram.addAttribute("vertexPosition");
	m_textureProgram.addAttribute("vertexColor");
	m_textureProgram.addAttribute("vertexUV");
	m_textureProgram.linkShaders();
}

void MainGame::gameLoop() {

	//helpful constants
	const float DESIRED_FPS = 60.0f;
	const int MAX_PHYSICS_STEPS = 6;
	const float MS_PER_SECOND = 1000;
	const float DESIRED_FRAMETIME = MS_PER_SECOND / DESIRED_FPS;
	const float MAX_DELTA_TIME = 1.0f;

	//Cap the FPS
	GameEngine::FpsLimiter fpsLimiter;
	fpsLimiter.setMaxFPS(600000.0f);

	//Zoom out the camera by 3x
	const float CAMERA_SCALE = 1.0f / 3.0f;
	m_camera.setScale(CAMERA_SCALE);

	float previousTicks = SDL_GetTicks();

	while (m_gameState == GameState::PLAY)
	{
		float newTicks = SDL_GetTicks();
		float frameTime = newTicks - previousTicks;
		previousTicks = newTicks;
		float totalDeltaTime = frameTime / DESIRED_FRAMETIME;

		fpsLimiter.beginFrame();

		checkVictory();

		m_inputManager.update();

		processInput();

		int i = 0;
		while (totalDeltaTime > 0.0f && i < MAX_PHYSICS_STEPS)
		{
			float deltaTime = std::min(totalDeltaTime, MAX_DELTA_TIME);
			updateAgents(deltaTime);

			updateBullets(deltaTime);

			m_particleEngine.update(deltaTime);

			totalDeltaTime -= deltaTime;
			i++;
		}

		m_camera.setPosition(m_player->getPosition());

		m_camera.update();

		m_hudCamera.update();

		drawGame();

		m_fps = fpsLimiter.end();
		std::cout << m_fps << std::endl;
	}
}

void MainGame::updateAgents(float deltaTime){
	//Update all humans
	for (int i = 0; i < m_humans.size(); i++)
	{
		m_humans[i]->update(m_levels[m_currentLevel]->getLevelData(), m_humans, m_zombies, deltaTime);
	}

	//Update all zombies
	for (int i = 0; i < m_zombies.size(); i++)
	{
		m_zombies[i]->update(m_levels[m_currentLevel]->getLevelData(), m_humans, m_zombies, deltaTime);
	}

	//Update zombie collisions
	for (int i = 0; i < m_zombies.size(); i++)
	{
		//Collide with other zombies
		for (int j = i + 1; j < m_zombies.size(); j++) {
			m_zombies[i]->collideWithAgent(m_zombies[j]);
		}

		//Collide with humans (player is zero so skip him)
		for (int j = 1; j < m_humans.size(); j++) {
			if (m_zombies[i]->collideWithAgent(m_humans[j])){
				//Add the new zombie
				m_zombies.push_back(new Zombie);
				m_zombies.back()->init(ZOMBIE_SPEED, m_humans[j]->getPosition());
				//Delete the human
				delete m_humans[j];
				m_humans[j] = m_humans.back();
				m_humans.pop_back();
			}
		}

		//Collide with player
		if (m_zombies[i]->collideWithAgent(m_humans[0])){
			std::printf("Killed by zombie %d with health: %d at Pos: %d, %d \n", i, m_zombies[i]->getHealth(), m_zombies[i]->getPosition().x, m_zombies[i]->getPosition().y);
			GameEngine::fatalError("YOU LOSE");
		}
	}

	//Update human collisions
	for (int i = 0; i < m_humans.size(); i++)
	{
		//Collide with other humans
		for (int j = i + 1; j < m_humans.size(); j++) {
			m_humans[i]->collideWithAgent(m_humans[j]);
		}
	}
}

void MainGame::updateBullets(float deltaTime){
	//Update and collide with world
	for (int i = 0; i < m_bullets.size();)
	{
		//if update returns true, the bullet collided with a wall
		if (m_bullets[i].update(m_levels[m_currentLevel]->getLevelData(), deltaTime)){
			m_bullets[i] = m_bullets.back();
			m_bullets.pop_back();
		}
		else{
			i++;
		}
	}

	bool wasBulletRemoved;

	//Collide with agents (humans and zombies)
	for (int i = 0; i < m_bullets.size(); i++)
	{
		wasBulletRemoved = false;
		//Loop through zombies
		for (int j = 0; j < m_zombies.size();)
		{
			//Check collision
			if (m_bullets[i].collideWithAgent(m_zombies[j])){
				//Add blood first
				addBlood(m_bullets[i].getPosition(), 5);

				//Damage zombie and kill it if its out of health
				if (m_zombies[j]->applyDamage(m_bullets[i].getDamage())){
					//If the zombie died, remove it
					delete m_zombies[j];
					m_zombies[j] = m_zombies.back();
					m_zombies.pop_back();
					m_numZombiesKilled++;
				}
				else{
					j++;
				}

				//Remove the bullet 
				m_bullets[i] = m_bullets.back();
				m_bullets.pop_back();

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
			for (int j = 1; j < m_humans.size();)
			{
				if (m_bullets[i].collideWithAgent(m_humans[j])){
					//Add blood first
					addBlood(m_bullets[i].getPosition(), 5);

					//Damage human and kill it if its out of health
					if (m_humans[j]->applyDamage(m_bullets[i].getDamage())){
						//If the human died, remove it
						delete m_humans[j];
						m_humans[j] = m_humans.back();
						m_humans.pop_back();
						m_numHumansKilled++;
					}
					else{
						j++;
					}

					//Remove the bullet 
					m_bullets[i] = m_bullets.back();
					m_bullets.pop_back();

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
	// m_currentLevel++; initLevel(...);
	//If all zombies are dead we win!
	if (m_zombies.empty()){

		std::printf("*** YOU WIN! ***\n You killed %d humans and %d zombies. There are %d out of %d civilians remaining.", m_numHumansKilled, m_numZombiesKilled, m_humans.size() - 1, m_levels[m_currentLevel]->getNumHumans());

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
			m_gameState = GameState::EXIT;
			break;
		case SDL_MOUSEMOTION:
			m_inputManager.setMouseCoords(evnt.motion.x, evnt.motion.y);
			break;
		case SDL_KEYDOWN:
			m_inputManager.pressKey(evnt.key.keysym.sym);
			break;
		case SDL_KEYUP:
			m_inputManager.releaseKey(evnt.key.keysym.sym);
			break;
		case SDL_MOUSEBUTTONDOWN:
			m_inputManager.pressKey(evnt.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			m_inputManager.releaseKey(evnt.button.button);
			break;
		}
	}
}

void MainGame::drawGame() {
	// Set the base depth to 1.0
	glClearDepth(1.0);
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_textureProgram.use();

	//Draw code goes here
	glActiveTexture(GL_TEXTURE0);

	//Make sure the shader uses texture 0
	GLint textureUniform = m_textureProgram.getUniformLocation("mySampler");
	glUniform1i(textureUniform, 0);

	//Grab the camera matrix
	glm::mat4 projectionMatrix = m_camera.getCameraMatrix();
	GLint pUniform = m_textureProgram.getUniformLocation("P");
	glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

	//Draw the level
	m_levels[m_currentLevel]->draw();

	//Begin drawing agents
	m_agentSpriteBatch.begin();

	const glm::vec2 agentDims(AGENT_RADIUS * 2.0f);

	//Draw the humans
	for (int i = 0; i < m_humans.size(); i++)
	{
		if (m_camera.isBoxInView(m_humans[i]->getPosition(), agentDims))
		{
			m_humans[i]->draw(m_agentSpriteBatch);
		}
	}

	//Draw the zombies
	for (int i = 0; i < m_zombies.size(); i++)
	{
		if (m_camera.isBoxInView(m_zombies[i]->getPosition(), agentDims))
		{
			m_zombies[i]->draw(m_agentSpriteBatch);
		}
	}

	//Draw the bullets
	for (int i = 0; i < m_bullets.size(); i++)
	{
		m_bullets[i].draw(m_agentSpriteBatch);
	}

	m_agentSpriteBatch.end();

	m_agentSpriteBatch.renderBatch();

	//Render the particles
	m_particleEngine.draw(&m_agentSpriteBatch);

	//Render the heads up display
	drawHUD();

	m_textureProgram.unuse();

	// Swap our buffer and draw everything to the screen!
	m_window.swapBuffer();
}

void MainGame::drawHUD(){
	char buffer[256];

	//Grab the hud camera matrix
	glm::mat4 projectionMatrix = m_hudCamera.getCameraMatrix();
	GLint pUniform = m_textureProgram.getUniformLocation("P");
	glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

	m_hudSpriteBatch.begin();

	sprintf_s(buffer, "Num Humans %d", m_humans.size());
	m_spriteFont->draw(m_hudSpriteBatch, buffer, glm::vec2(0, 0), glm::vec2(0.5), 0.0f, GameEngine::ColorRGBA8(255, 255, 255, 255));

	sprintf_s(buffer, "Num Zombies %d", m_zombies.size());
	m_spriteFont->draw(m_hudSpriteBatch, buffer, glm::vec2(0, 36), glm::vec2(0.5), 0.0f, GameEngine::ColorRGBA8(255, 255, 255, 255));

	m_hudSpriteBatch.end();

	m_hudSpriteBatch.renderBatch();
}

void MainGame::addBlood(glm::vec2& position, int numParticles){



	static std::mt19937 randEngine(time(nullptr));
	static std::uniform_real_distribution <float> randAngle(0.0f, 360.0f * DEG_TO_RAD);

	glm::vec2 velocity(2.0f, 0.0f);

	GameEngine::ColorRGBA8 color(255, 0, 0, 255);

	for (int i = 0; i < numParticles; i++)
	{
		m_bloodParticleBatch->addParticle(position, glm::rotate(velocity, randAngle(randEngine)), color, 30.0f);
	}

}