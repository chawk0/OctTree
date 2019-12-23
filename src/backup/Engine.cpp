#include "Engine.h"

/*
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
		*/

Engine::Engine(void):
	_glContext(NULL),
	_keyStates(NULL),
	_initFunc(NULL),
	_inputFunc(NULL),
	_renderFunc(NULL),
	_cleanupFunc(NULL),
	_currentCamera(NULL),
	_currentShader(NULL)
{
	//
}

Engine::~Engine(void)
{
	// hmm?
	//cleanup();
}

void Engine::setCallbacks(InitCallback initFunc, InputCallback inputFunc, RenderCallback renderFunc, CleanupCallback cleanupFunc)
{
	_initFunc = initFunc;
	_inputFunc = inputFunc;
	_renderFunc = renderFunc;
	_cleanupFunc = cleanupFunc;
}

int Engine::init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
	{
		printf("error: failed to init SDL\n\t%s\n\n", SDL_GetError());
		exit(1);
	}
 
    /* Request opengl 3.3 context.
     * SDL doesn't have the ability to choose which profile at this time of writing,
     * but it should default to the core profile */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
 
    /* Turn on double buffering with a 24bit Z buffer.
     * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
 
    /* Create our window centered at 512x512 resolution */
    _window = SDL_CreateWindow("jesus christ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1440, 900, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!_window) /* Die if creation failed */
	{
		cout << "failed to create SDL window :(" << endl;
		return -1;
	}
 
    /* Create our opengl context and attach it to our window */
    _glContext = SDL_GL_CreateContext(_window);
	if (_glContext == NULL)
	{
		cout << "error: failed to create GL context\n\t" << SDL_GetError() << endl;
		return -1;
	}

	SDL_GL_SetSwapInterval(0);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	glewExperimental = GL_TRUE; 
	glewInit();
	glGetError();	// glew fires a 1280. clear it
	
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
	//glDepthRange(0.0f, 1.0f);
	glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	// prime the RNG!
	time_t seed;
	time(&seed);
	srand((unsigned long)seed);

	// init the high precision Timer object
	_timer.init();

	//world.create("./res/segments/world.txt");
	//world.temp();

	if (_initFunc != NULL)
		_initFunc();

	return 0;
}

int Engine::run()
{
	bool done = false;
	SDL_Event event;
	double dt;
	float mouseDX, mouseDY;
	int frameCount = 0;
	char title[80];

	while (!done)
    {
        while (SDL_PollEvent(&event) == 1)
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    done = true;
                    break;

                case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE)
						done = true;
                    break;

				case SDL_MOUSEMOTION:
					if (_currentCamera != NULL)
					{
						mouseDX = (float)event.motion.xrel / 1440.0f;
						mouseDY = (float)event.motion.yrel / 900.0f;
						_currentCamera->rotateX(mouseDY * 135.0f);
						_currentCamera->rotateY(mouseDX * 135.0f);
					}
					break;
            }
        }

		dt = _timer.getElapsedSeconds();
		if (_inputFunc != NULL)
			_inputFunc(dt);
		
		//updateShader();
		//_currentShader->setUniform1i("useTexturing", 1);
		//world.render();
		if (_renderFunc != NULL)
		{
			_renderFunc(dt);
			SDL_GL_SwapWindow(_window);
		}
		
		frameCount++;
		if (frameCount == 1000)
		{
			frameCount = 0;
			sprintf_s(title, 80, "jesus christ, fps: %d", (unsigned int)_timer.getFPS(1000));
			SDL_SetWindowTitle(_window, title);
		}
    }

	return 0;
}

void Engine::cleanup()
{
	for (auto i = _cameras.begin(); i != _cameras.end(); ++i)
		delete i->second;
	_cameras.clear();
	_currentCamera = NULL;

	for (auto i = _shaders.begin(); i != _shaders.end(); ++i)
		delete i->second;
	_shaders.clear();
	_currentShader = NULL;

	if (_cleanupFunc != NULL)
		_cleanupFunc();

	SDL_SetRelativeMouseMode(SDL_FALSE);
	SDL_Quit();
}

Camera* Engine::createCamera(const char* name)
{
	Camera* newCamera = new (nothrow) Camera();
	
	// if a camera already exists with the given name, delete it first
	auto i = _cameras.find(string(name));
	if (i != _cameras.end())
	{
		delete i->second;
		_cameras.erase(i);
	}

	// if there's only this one camera in the map, set it to the current
	if (_cameras.size() == 0)
		_currentCamera = newCamera;

	// add it
	_cameras[string(name)] = newCamera;

	return newCamera;
}

void Engine::bindCamera(const char* name)
{
	auto i = _cameras.find(string(name));
	if (i != _cameras.end())
		_currentCamera = i->second;
}

void Engine::destroyCamera(const char* name)
{
	auto i = _cameras.find(string(name));
	if (i != _cameras.end())
	{
		delete i->second;
		_cameras.erase(i);
	}

	// if there're no more cameras, NULL out the current pointer
	if (_cameras.size() == 0)
		_currentCamera = NULL;
}

Camera* Engine::getCamera()
{
	return _currentCamera;	
}

Camera* Engine::getCamera(const char* name)
{
	auto i = _cameras.find(string(name));
	if (i != _cameras.end())
		return i->second;
	else
		return NULL;
}

Shader* Engine::createShader(const char* name, const char* vp, const char* fp)
{
	Shader* newShader;

	// if a shader already exists with the given name, delete it first
	auto i = _shaders.find(string(name));
	if (i != _shaders.end())
	{
		delete i->second;
		_shaders.erase(i);
	}

	newShader = new (nothrow) Shader();
	newShader->create(vp, fp);
	_shaders[string(name)] = newShader;
	return newShader;
}

void Engine::updateShader()
{
	// update the current shader (if there is one) with the WVP and world matrices
	// if there's also a camera
	if (_currentShader != NULL && _currentCamera != NULL)
	{
		Matrix4 t, wvp;

		// the full transform is proj * view * -translate(cam_pos) * world
		t.makeTranslate(-_currentCamera->getPosition());
		wvp = _currentCamera->getProjectionMatrix() * _currentCamera->getViewMatrix() * t * _worldMatrixStack.top();

		// update the shader
		_currentShader->setMatrix4f("WVPMatrix", wvp);
		_currentShader->setMatrix4f("worldMatrix", _worldMatrixStack.top());
	}
}

void Engine::bindShader(const char* name)
{
	auto i = _shaders.find(string(name));
	if (i != _shaders.end())
	{
		_currentShader = i->second;
		_currentShader->use();
	}
}

void Engine::destroyShader(const char* name)
{
	auto i = _shaders.find(string(name));
	if (i != _shaders.end())
	{
		delete i->second;
		_shaders.erase(i);
	}
}

Shader* Engine::getShader()
{
	return _currentShader;
}

Shader* Engine::getShader(const char* name)
{
	auto i = _shaders.find(string(name));
	if (i != _shaders.end())
		return i->second;
	else
		return NULL;
}

MatrixStack* Engine::getWorldMatrix()
{
	return &_worldMatrixStack;
}