#pragma once

#include <Windows.h>

#include <SDL/SDL.h>
#include <GL/glew.h>

#include <vector>

#include <GameEngine/GameEngine.h>
#include <GameEngine/GLSLProgram.h>
#include <GameEngine/GLTexture.h>
#include <GameEngine/Sprite.h>
#include <GameEngine/Window.h>
#include <GameEngine/Camera2D.h>
#include <GameEngine/SpriteBatch.h>
#include <GameEngine/InputManager.h>
#include <GameEngine/Timing.h>

enum class GameState{PLAY, EXIT};


class MainGame
	{
	public:
		MainGame(void);
		~MainGame(void);

		void run();

	
	private:
		void initSystems();
		void initShaders();
		void gameLoop();
		void processInput();
		void drawGame();

		GameEngine::Window _window;
		int _screenWidth;
		int _screenHeight;

		GameState _gameState;

		GameEngine::GLSLProgram _colorProgram;
		GameEngine::GLTexture _playerTexture;

		GameEngine::Camera2D _camera;

		GameEngine::SpriteBatch _spriteBatch;

		GameEngine::InputManager _inputManager;

		GameEngine::FpsLimiter _fpsLimiter;

		float _fps;
		float _maxFPS;

		float _time;
	};

