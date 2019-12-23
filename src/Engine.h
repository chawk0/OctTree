/*
	The Engine class basically maintains all of the other class objects, facilitates communication between them, and handles the state of the game world.
*/

#pragma once
#include "ChawkFortress.h"

#include "Array.h"
#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "PhysicsEngine.h"
#include "Shader.h"
#include "Font.h"
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

	// basic stuff!
	void setCallbacks(InitCallback initFunc, InputCallback inputFunc, RenderCallback renderFunc, CleanupCallback cleanupFunc);
	int init(int windowWidth, int windowHeight);
	int run();
	void cleanup();
	int getWindowWidth();
	int getWindowHeight();

	// interface between game data and engine
	void loadVoxelDefs(const char* file);
	Array<Voxel>* getVoxelDefs();
	void createWorld(const char* file);
	void buildOctree(uint threshold);
	void destroyWorld();

	// camera functions
	Camera* createCamera(const char* name);
	void bindCamera(const char* name);
	void destroyCamera(const char* name);
	Camera* getCamera();
	Camera* getCamera(const char* name);

	// shader functions
	Shader* createShader(const char* name, const char* vp, const char* fp);
	void updateShaderMatrices();
	Shader* bindShader(const char* name);
	void destroyShader(const char* name);
	Shader* getShader();
	Shader* getShader(const char* name);

	// font functions
	Font* createFont(const string& name, const string& fontPath, int pixelSize);
	Font* bindFont(const string& name);
	void setFontColor(float r, float g, float b);
	void renderTextf(float x, float y, const string& format, ...);
	void destroyFont(const string& name);


	// allow "client" code to directly modify this
	MatrixStack* getWorldMatrix();

	World world;
//private:
	// window stuff and
	SDL_Window* _window;
	int _windowWidth, _windowHeight;
	SDL_GLContext _glContext;
	unsigned char* _keyStates;

	// "client" callbacks
	InitCallback _initFunc;
	InputCallback _inputFunc;
	RenderCallback _renderFunc;
	CleanupCallback _cleanupFunc;
	// internal engine timer
	Timer _timer;

	// internal list of cameras, shaders and fonts
	unordered_map<string, Camera*> _cameras;
	Camera* _currentCamera;
	unordered_map<string, Shader*> _shaders;
	Shader* _currentShader;
	unordered_map<string, Font*> _fonts;
	Font* _currentFont;

	Matrix4 _orthoMatrix;

	// the world matrix is maintained by the engine, while the view and projection are
	// properties of the current camera being rendered from.  the combination into a WVP
	// is done in the updateShader method.
	MatrixStack _worldMatrixStack;

	// voxel definitions used by World and Octree. the id of a voxel is the same as its
	// index into this array.  the voxels are created in the order they're read from 
	// the provided voxeldef file in loadVoxelDefs
	Array<Voxel> _voxelDefs;

	// zee verld!
	World _world;

	// zee achtree!
	Octree _octree;

	// ayup
	PhysicsEngine _physics;
};

