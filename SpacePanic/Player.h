#pragma once

#include "Box.h"
#include <GameEngine\SpriteBatch.h>
#include <GameEngine\GLTexture.h>
#include <GameEngine\InputManager.h>
#include <GameEngine\AudioEngine.h>

#include "Agent.h"
#include "Monster.h"
#include "Level.h"

class Player :
	public Agent
{
public:
	Player();
	~Player();

	void init(GameEngine::InputManager* inputManager, const glm::vec2 position, const glm::vec2 dimensions, std::string textureFilePath, GameEngine::ColorRGBA8 color, float speed, GameEngine::SoundEffect digHoleSound, GameEngine::SoundEffect closeHoleSound, GameEngine::SoundEffect dyingSound);

	virtual void update(const std::vector<std::string>& levelData, std::vector<Player*>& players, std::vector<Monster*>& monsters, float deltaTime) override;

	virtual void update(std::vector<Box>& levelBoxes, std::vector<Player*>& players, std::vector<Monster*>& monsters, float deltaTime) override;
	virtual void update(Level& level, std::vector<Player*>& players, std::vector<Monster*>& monsters, float deltaTime) override;

	virtual bool collideWithHalfHole(std::vector<Box>& levelBoxes) override;
	virtual bool collideWithHole(std::vector<Box>& levelBoxes) override;

	void draw(GameEngine::SpriteBatch& spriteBatch);

	void kill();

	void addPoints(int points){
		m_gamePoints += points * m_consecutiveMonsterKills;
	}

	int getPoints() const {
		return m_gamePoints;
	}

	void addConsecutiveMonsterKills() { m_consecutiveMonsterKills = m_consecutiveMonsterKills + 1; }
	int getConsecutiveMonsterKills() const { return m_consecutiveMonsterKills; }

	bool isAlive(){
		return m_isAlive;
	}

private:

	void updateMovements(Level& level, std::vector<Player*>& players, float deltaTime);
	void updateActions(Level& level, std::vector<Player*>& players, std::vector<Monster*>& monsters, float deltaTime);
	bool tryDigging(Level& level, std::vector<Player*>& players, std::vector<Monster*>& monsters, Box& groundBox);
	void playDiggingSound();
	void playCloseHoleSound();

	void fallThroughHole(Box& holeBox);

	GameEngine::InputManager* m_inputManager;

	int m_gamePoints = 0;

	int m_consecutiveMonsterKills = 0; //consecutive Monster Kills for one round

	bool m_isAlive = true;

	GameEngine::SoundEffect m_digHoleSound; 
	GameEngine::SoundEffect m_closeHoleSound; 
	GameEngine::SoundEffect m_dyingSound;

};

