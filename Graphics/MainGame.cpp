#include "MainGame.h"


#include <iostream>
#include <string>

#include <GameEngine/Errors.h>




MainGame::MainGame(void) : 
	_screenWidth(1024),   //Initialzation list
	_screenHeight(768), 
	_time(0),
	_gameState(GameState::PLAY),
	_maxFPS(60.0f)
	{
	_camera.init(_screenWidth, _screenHeight);
	}


MainGame::~MainGame(void)
	{
	}

void MainGame::run(){
	initSystems();

	//fix it later
	_sprites.push_back(new GameEngine::Sprite());
	_sprites.back()->init(0.0f, 0.0f, _screenWidth/2, _screenWidth/2, "Textures/JimmyJump/PNG/CharacterRight_Standing.png");

	_sprites.push_back(new GameEngine::Sprite());
	_sprites.back()->init(_screenWidth/2, 0.0f,_screenWidth/2, _screenWidth/2, "Textures/JimmyJump/PNG/CharacterRight_Standing.png");

	//for (int i = 0; i < 1000; i++){
	//_sprites.push_back(new GameEngine::Sprite());
	//_sprites.back()->init(-1.0f, 0.0f, 1.0f, 1.0f, "Textures/JimmyJump/PNG/CharacterRight_Standing.png");
	//	}


	//_playerTexture = ImageLoader::loadPNG("Textures/JimmyJump/PNG/CharacterRight_Standing.png");

	gameLoop();
	}



void MainGame::initSystems(){
	GameEngine::init();

	_window.create("Game Engine", _screenWidth, _screenHeight, 0);

	initShaders();
	}


void MainGame::initShaders(){
	_colorProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
	_colorProgram.addAttribute("vertexPosition");
	_colorProgram.addAttribute("vertexColor");
	_colorProgram.addAttribute("vertexUV");
	_colorProgram.linkShaders();
	}


void MainGame::gameLoop(){

	//loop until the game state is EXIT
	while (_gameState != GameState::EXIT ){
		//Used for frame time measuring
		float startTicks = SDL_GetTicks();

		processInput();
		_time += 0.01;

		_camera.update();

		drawGame();
		calculateFPS();

		//print only once every 10 frames
		static int frameCounter = 0;
		frameCounter++;
		if (frameCounter == 10){
			std::cout << _fps << std::endl;
			frameCounter = 0;
			}

		float frameTicks = SDL_GetTicks() - startTicks;
		//Limit the FPS to the max FPS value 
		if (1000.0f / _maxFPS > frameTicks){
			SDL_Delay(1000.0f / _maxFPS - frameTicks);
			}
		}

	}


void MainGame::processInput(){
	SDL_Event evnt;

	const float CAMERA_SPEED = 20.0f;
	const float SCALE_SPEED = 0.1f;

	while(SDL_PollEvent(&evnt) == true){
		switch (evnt.type){
			case SDL_QUIT:
				_gameState = GameState::EXIT;
				break;
			case SDL_MOUSEMOTION:
				//std::cout << evnt.motion.x << " " << evnt.motion.y  << std::endl;
				break;
			case SDL_KEYDOWN:
				switch (evnt.key.keysym.sym){
					case SDLK_w:
						_camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, CAMERA_SPEED));
						break;
					case SDLK_s:
						_camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, -CAMERA_SPEED));
						break;
					case SDLK_a:
						_camera.setPosition(_camera.getPosition() + glm::vec2(CAMERA_SPEED, 0.0f));
						break;
					case SDLK_d:
						_camera.setPosition(_camera.getPosition() + glm::vec2(-CAMERA_SPEED, 0.0f));
						break;
					case SDLK_q:
						_camera.setScale(_camera.getScale() + SCALE_SPEED);
						break;
					case SDLK_e:
						_camera.setScale(_camera.getScale() - SCALE_SPEED);
						break;
					}
				break;
			}
		}
	}

void MainGame::drawGame() {

	//Set the base depth to 1.0
	glClearDepth(1.0);
	//clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // same as callinglClear fot both variables


	_colorProgram.use();
	glActiveTexture(GL_TEXTURE0);
	GLint textureLocation = _colorProgram.getUniformLocation("mySampler");
	glUniform1i(textureLocation, 0);

	GLuint timeLocation = _colorProgram.getUniformLocation("time");
	glUniform1f(timeLocation, _time);

	//set the camera matrix
	GLuint pLocation = _colorProgram.getUniformLocation("P");
	glm::mat4& cameraMatrix = _camera.getCameraMatrix();
	glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(cameraMatrix[0][0]));


	//Draw our sprites!
	for (int i = 0; i < _sprites.size(); i++)
		{
		_sprites[i]->draw();
		}

	glBindTexture(GL_TEXTURE_2D, 0);


	_colorProgram.unuse();

	//Swap our buffer and draw everything to the screen
	_window.swapBuffer();
	}


void MainGame::calculateFPS(){
	static const int NUM_SAMPLES = 10;
	static float frameTimes[NUM_SAMPLES];
	static int currentFrame = 0;

	static float prevTicks = SDL_GetTicks();

	float currentTicks;
	currentTicks = SDL_GetTicks();

	_frameTime = currentTicks - prevTicks;
	frameTimes[currentFrame % NUM_SAMPLES] = _frameTime;

	prevTicks = currentTicks;

	int count; 

	currentFrame++;
	if (currentFrame < NUM_SAMPLES){
		count = currentFrame;
		} else {
			count = NUM_SAMPLES;
		}

	float frameTimeAverage = 0;
	for (int i = 0; i < count; i++)
		{
		frameTimeAverage += frameTimes[i];
		}
	frameTimeAverage /= count;

	if (frameTimeAverage > 0){
		_fps = 1000.0f / frameTimeAverage;
		} else {
			_fps =  60.0f;
		}

	}