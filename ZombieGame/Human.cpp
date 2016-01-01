#include "Human.h"
#include <SDL\SDL.h>
#include <glm/glm.hpp>
#include <random>
#include <ctime>
#include <glm\gtx\rotate_vector.hpp>

const float DEG_TO_RAD = M_PI / 180.0f;
const float RAD_TO_DEG = 180.0f / M_PI;


//
//Human::Human(int speed, glm::vec2 position, glm::vec2 direction)
//{
//}

Human::Human() : _frames(0) {
	_health = 20;
}


Human::~Human()
{
}


void Human::init(float speed, glm::vec2 position){

	static std::mt19937 randomEngine(time(nullptr));
	static std::uniform_real_distribution<float> randDir(-1.0f, 1.0f);

	_color.r = 200;
	_color.g = 0;
	_color.b = 200;
	_color.a = 255;

	_speed = speed;
	_position = position;
	//Get random direction
	_direction = glm::vec2(randDir(randomEngine), randDir(randomEngine));

	//Make sure direction isn't zero
	if (_direction.length() == 0){
		_direction = glm::vec2(1.0f, 0.0f);
	}

	_direction = glm::normalize(_direction);
}


void Human::update(const std::vector<std::string>& levelData,
	std::vector<Human*>& humans, std::vector<Zombie*>& zombies, float deltaTime){

	static std::mt19937 randomEngine(time(nullptr));
	static std::uniform_real_distribution<float> randRotate(-40.0f * DEG_TO_RAD, 40.0f * DEG_TO_RAD);
	_position += _direction * _speed * deltaTime; 

	//Randomly change direction every 20 frames
	if (_frames == 20){
		_direction = glm::rotate(_direction, randRotate(randomEngine));
		_frames = 0;
	}
	else
	{
		_frames++;
	}

	if (collideWithLevel(levelData)){
		_direction = glm::rotate(_direction, randRotate(randomEngine));
	}
}