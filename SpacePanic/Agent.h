#pragma once

#include <SDL\SDL.h>
#include <glm/glm.hpp>
//#include <GameEngine\GLTexture.h>
#include <GameEngine\SpriteBatch.h>

#include "Box.h"

const float AGENT_WIDTH = 60;
const float AGENT_RADIUS = AGENT_WIDTH / 2.0f;

class Monster;
class Player;
class Level;

class Agent
{
public:

	Agent();
	virtual ~Agent();
	/*explicit
		Agent(float speed, glm::vec2 position, glm::vec2 direction)
		: m_speed(speed), m_direction(direction), m_position(position)
		{}
		*/

	void draw(GameEngine::SpriteBatch& spriteBatch);

	virtual void update(const std::vector<std::string>& levelData,
		std::vector<Player*>& players, std::vector<Monster*>& monsters, float deltaTime) = 0;

	virtual void update(std::vector<Box>& levelBoxes, std::vector<Player*>& players, std::vector<Monster*>& monsters, float deltaTime) = 0;

	virtual void update(Level& level, std::vector<Player*>& players, std::vector<Monster*>& monsters, float deltaTime) = 0;

	virtual bool collideWithHalfHole(std::vector<Box>& levelBoxes) = 0;
	virtual bool collideWithHole(std::vector<Box>& levelBoxes) = 0;

	glm::vec2 getPosition() const { return m_collisionBox.getPosition(); }
	void setPosition(glm::vec2 newPosition) { m_collisionBox.m_position = newPosition; }

	bool collideWithLevel(const std::vector<std::string>& levelData);
	bool collideWithLevel(std::vector<Box>& levelBoxes);
	bool collideWithLadder(std::vector<Box>& ladderBoxes);

	bool collideWithAgent(Agent* agent, glm::vec4& penetrationDepth = glm::vec4());

	//Return true if health is zero or less --> agent died
	bool applyDamage(float damage);

	//int getID() {
	//	return m_id;
	//};

	virtual float getHealth() {
		return m_health;
	};

	virtual void setHealth(float health) {
		m_health = health;
	};

	const Box& getBox() const { return m_collisionBox; }

	glm::vec2 getDirection() const { return m_direction; }

protected:

	void checkTilePosition(const std::vector<std::string>& levelData,
		std::vector<glm::vec2>& collideTilePosition,
		float x, float y);

	bool collideWithTile(glm::vec2 tilePosition);

	bool collideWithTile(glm::vec2 tilePosition, glm::vec2 tileDimensions);

	bool collideWithBox(const Box* otherBox, glm::vec4& penetrationDepth);

	bool collideBoxWithBox(const Box& box, const Box& otherBox);
	bool collideBoxWithBox(const Box& box, const Box& otherBox, glm::vec4& penetrationDepth);
	bool collideBoxWithBoxes(Box& box, std::vector<Box>& boxes);

	Box* collideWithLadderAndGetLadderBox(std::vector<Box>& ladderBoxes);
	glm::vec4 collideWithLadderAndGetCollisionDepth(std::vector<Box>& ladderBoxes);

	void handleCollisionWithUnmoveableObject(glm::vec4 penetrationDepth);

	bool isSameBox(Box* box, Box* otherBox);
	bool isInAir(std::vector<Box>& levelBoxes);
	bool canWalkForward(std::vector<Box>& levelBoxes, Box& groundBox, Box& boxAboveGround);

	bool halfHoleAhead(std::vector<Box>& halfHoleBoxes, Box& groundBox);
	bool holeAhead(std::vector<Box>& holeBoxes, Box& groundBox);

	bool m_onLadder = false;

	glm::vec2 m_direction = glm::vec2(1.0f, 0.0f);

	float m_speed;
	float m_health;

	Box m_collisionBox;




};

