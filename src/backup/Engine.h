/*
	The Engine class basically maintains all of the other class objects, facilitates communication between them, and handles the state of the game world.

	Nice.
*/

#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Camera.h"
//#include "Log.h"
#include "MatrixStack.h"
//#include "Perlin.h"
//#include "SceneObject.h"
#include "Shader.h"
//#include "Sketch.h"
//#include "Texture.h"
#include "Timer.h"
#include "World.h"

class Engine
{
	public:
		// function pointers for "client-side" functions that the engine will call
		typedef void (*InitCallback)(void);
		typedef void (*InputCallback)(double);
		typedef void (*RenderCallback)(double);
		typedef void (*CleanupCallback)(void);

		Engine();
		~Engine();

		void setCallbacks(InitCallback initFunc, InputCallback inputFunc, RenderCallback renderFunc, CleanupCallback cleanupFunc);
		int init();
		int run();
		void cleanup();

		// camera functions
		Camera* createCamera(const char* name);
		void bindCamera(const char* name);
		void destroyCamera(const char* name);
		Camera* getCamera();
		Camera* getCamera(const char* name);

		// shader functions
		Shader* createShader(const char* name, const char* vp, const char* fp);
		void updateShader();
		void bindShader(const char* name);
		void destroyShader(const char* name);
		Shader* getShader();
		Shader* getShader(const char* name);

		// world matrix functions
		MatrixStack* getWorldMatrix();

		World world;
	private:
		SDL_Window* _window;
		SDL_GLContext _glContext;
		unsigned char* _keyStates;

		InitCallback _initFunc;
		InputCallback _inputFunc;
		RenderCallback _renderFunc;
		CleanupCallback _cleanupFunc;
		Timer _timer;

		unordered_map<string, Camera*> _cameras;
		Camera* _currentCamera;
		unordered_map<string, Shader*> _shaders;
		Shader* _currentShader;

		MatrixStack _worldMatrixStack;		
};

