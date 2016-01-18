#include "Monster.h"

#include "Player.h"

#include <GameEngine\ResourceManager.h>

Monster::Monster()
{
	m_health = 150;
}


Monster::~Monster()
{

}

void Monster::init(float speed, glm::vec2 position, glm::vec2 dimensions){
	m_collisionBox.m_color = GameEngine::ColorRGBA8(255, 255, 255, 255);
	m_speed = speed;
	m_collisionBox.m_position = position;
	m_collisionBox.m_dimensions = dimensions;
	m_collisionBox.m_textureID = GameEngine::ResourceManager::getTexture("Textures/zombie.png").id;

}

void Monster::update(const std::vector<std::string>& levelData,
	std::vector<Player*>& Players, std::vector<Monster*>& zombies, float deltaTime){

	Player* closestPlayer = getNearestPlayer(Players);

	if (closestPlayer != nullptr){
		m_direction = glm::normalize(closestPlayer->getPosition() - m_collisionBox.m_position);
		m_collisionBox.m_position += m_direction * m_speed * deltaTime;
	}

	collideWithLevel(levelData);

}

Player* Monster::getNearestPlayer(std::vector<Player*>& Players){
	Player* closestPlayer = nullptr;
	float smallestDistance = 999999999999.0f;

	for (int i = 0; i < Players.size(); i++)
	{
		glm::vec2 distVec = Players[i]->getPosition() - m_collisionBox.m_position;
		float distance = glm::length(distVec);

		if (distance < smallestDistance){
			smallestDistance = distance;
			closestPlayer = Players[i];
		}
	}

	return closestPlayer;
}

