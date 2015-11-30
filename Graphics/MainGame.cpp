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

	}


MainGame::~MainGame(void)
	{
	}

void MainGame::run(){
	initSystems();

	//fix it later
	_sprites.push_back(new GameEngine::Sprite());
	_sprites.back()->init(-1.0f, -1.0f, 1.0f, 1.0f, "Textures/JimmyJump/PNG/CharacterRight_Standing.png");

	_sprites.push_back(new GameEngine::Sprite());
	_sprites.back()->init(0.0f, -1.0f, 1.0f, 1.0f, "Textures/JimmyJump/PNG/CharacterRight_Standing.png");

	//for (int i = 0; i < 1000; i++){
	_sprites.push_back(new GameEngine::Sprite());
	_sprites.back()->init(-1.0f, 0.0f, 1.0f, 1.0f, "Textures/JimmyJump/PNG/CharacterRight_Standing.png");
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

	while(SDL_PollEvent(&evnt) == true){
		switch (evnt.type){
			case SDL_QUIT:
				_gameState = GameState::EXIT;
				break;
			case SDL_MOUSEMOTION:
				//std::cout << evnt.motion.x << " " << evnt.motion.y  << std::endl;
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