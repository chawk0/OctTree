#include "Engine.h"


Engine::Engine(void):
	_window(NULL),
	_glContext(NULL),
	_keyStates(NULL),
	_initFunc(NULL),
	_inputFunc(NULL),
	_renderFunc(NULL),
	_cleanupFunc(NULL),
	_currentCamera(NULL),
	_currentShader(NULL),
	_currentFont(NULL)
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

int Engine::init(int windowWidth, int windowHeight)
{
	cout << "initializing engine..." << endl;
	cout << "initializing SDL and OpenGL..." << endl;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
	{
		cout << "ERROR: Engine::init: failed to init SDL (" << SDL_GetError() << ")" << endl;
		return 1;
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
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
 
    /* Create our window centered at 512x512 resolution */
    _window = SDL_CreateWindow("jesus christ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!_window) /* Die if creation failed */
	{
		cout << "ERROR: Engine::init: failed to create SDL window (" << SDL_GetError() << endl;
		return 1;
	}

	_windowWidth = windowWidth;
	_windowHeight = windowHeight;

	// used for rendering fonts to screen space
	_orthoMatrix.makeOrtho(0.0f, static_cast<float>(_windowWidth), 0.0f, static_cast<float>(_windowHeight), 0.0f, 1.0f);
 
    /* Create our opengl context and attach it to our window */
    _glContext = SDL_GL_CreateContext(_window);
	if (_glContext == NULL)
	{
		cout << "ERROR: Engine::init: failed to create OpenGL context (" << SDL_GetError() << endl;
		return 1;
	}

	SDL_GL_SetSwapInterval(0);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	cout << "intializing glew and timer..." << endl;

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

	// prime the Perlin RNG (and rand() too)
	Perlin::init(static_cast<uint>(time(NULL)));
	//Perlin::init(0);

	// init the high precision Timer object
	_timer.init();

	// load the default font shaders
	createShader("fontShader", "./res/shaders/FontRender.vp", "./res/shaders/FontRender.fp");

	cout << "calling client init..." << endl << endl;

	// call the client callback
	if (_initFunc != NULL)
		_initFunc();

	return 0;
}

int Engine::getWindowWidth()
{
	return _windowWidth;
}

int Engine::getWindowHeight()
{
	return _windowHeight;
}

void Engine::loadVoxelDefs(const char* file)
{
	ifstream fileIn;
	string token;
	char buffer[261];
	
	Voxel newVoxel;
	uint currentVoxel = 1;
	uint count;
	bool inVoxel = false;

	cout << "loading voxel defs..." << endl;

	try
	{
		// load the voxel definitions
		fileIn.open(file, ios::in);

		while (!fileIn.eof() && !fileIn.fail())
		{
			token = "";
			fileIn >> token;

			if (token.length() > 0)
			{
				if (token[0] == '#')
					fileIn.getline(buffer, 260);
				else if (token == "voxels")
				{
					fileIn >> count;
					_voxelDefs.alloc(count + 1);
					// the first id, 0, is the null voxel and can't be used.
					_voxelDefs[0].name = "null";
					_voxelDefs[0].texture = NULL;
				}
				// start of a voxel def.
				else if (token == "voxel")
					inVoxel = true;
				else if (token == "name")
				{
					fileIn >> token;
					cout << "new voxel " << token << endl;
					newVoxel.name = token;
				}
				else if (token == "tex")
				{
					// max OS file path is around 260 characters
					fileIn.getline(buffer, 260);
					token = string(buffer);
					cout << "\tvoxel texture path: " << token << endl;
									
					// trim leading and trailing whitespace
					auto first = token.find_first_not_of(" \t");
					auto len = token.find_last_not_of(" \t") - first + 1;
					token = token.substr(first, len);
					newVoxel.texturePath = token;

					// create the texture object
					newVoxel.texture = new Texture();
					newVoxel.texture->create(token.c_str(), 0, 0, TCF_TEXTURE | TCF_MIPMAP | TCF_FORCE_32);
				}
				// once this token is reached, the current state of newVoxel is added as a new Voxel def to the vector
				else if (token == "end")
				{
					inVoxel = false;
					_voxelDefs[currentVoxel] = newVoxel;
					currentVoxel++;
				}
				else
					cout << "ERROR: Engine::loadVoxelDefs: unknown token: " << token << endl;
			}
		}

		fileIn.close();

		_world.setVoxelDefs(&_voxelDefs);
	}
	catch (exception e)
	{
		cout << "ERROR: Engine::loadVoxelDefs: exception loading voxel defs (" << e.what() << ")" << endl;
	}
}

Array<Voxel>* Engine::getVoxelDefs()
{
	return &_voxelDefs;
}

void Engine::createWorld(const char* file)
{
	cout << "creating world from " << file << "..." << endl;

	if (file != NULL)
	{
		if (_voxelDefs.size() > 0)
			_world.create(file);
		else
			cout << "ERROR: Engine::createWorld: no voxel defs to create from" << endl;
	}
}

void Engine::buildOctree(uint threshold)
{
	_octree.build(_world.getGrid(), _world.getGridSize(), _world.getSegments(), _world.getSegmentMap(), &_voxelDefs, _world.getSegmentRenderList(), threshold);
}

void Engine::destroyWorld()
{
	_world.destroy();
}

int Engine::run()
{
	bool done = false;
	SDL_Event event;
	double dt;
	float mouseDX, mouseDY;
	int frameCount = 0, fps;
	const int maxFrames = 60;
	char title[80];

	cout << "starting message loop..." << endl;

	_timer.reset();

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
						mouseDX = (float)event.motion.xrel / static_cast<float>(_windowWidth);
						mouseDY = (float)event.motion.yrel / static_cast<float>(_windowHeight);
						_currentCamera->rotateX(mouseDY * 135.0f);
						_currentCamera->rotateY(mouseDX * 135.0f);
					}
					break;
            }
        }

		// calls the double version
		dt = _timer.getElapsedSeconds2();
		if (_inputFunc != NULL)
			_inputFunc(dt);

		_physics.update(dt);
		
		if (_renderFunc != NULL)
		{
			_renderFunc(dt);
			SDL_GL_SwapWindow(_window);
		}
		
		frameCount++;
		if (frameCount == maxFrames)
		{
			frameCount = 0;
			fps = (int)_timer.getFPS2(maxFrames);
			sprintf_s(title, 80, "jesus christ, dt: %.9f, rendered leaves: %d, fps: %d", dt, _octree._lastFrameLeafRenderCount, fps);
			SDL_SetWindowTitle(_window, title);
		}
    }

	return 0;
}

void Engine::cleanup()
{
	cout << "cleaning up..." << endl;

	if (_cleanupFunc != NULL)
		_cleanupFunc();

	for (auto i = _cameras.begin(); i != _cameras.end(); ++i)
		delete i->second;
	_cameras.clear();
	_currentCamera = NULL;

	for (auto i = _shaders.begin(); i != _shaders.end(); ++i)
		delete i->second;
	_shaders.clear();
	_currentShader = NULL;

	for (auto i = _fonts.begin(); i != _fonts.end(); ++i)
		delete i->second;
	_fonts.clear();
	_currentFont = NULL;

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

void Engine::updateShaderMatrices()
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
		_currentCamera->getFrustum().extractPlanes(wvp);
		_currentShader->setMatrix4f("worldMatrix", _worldMatrixStack.top());
	}
}

Shader* Engine::bindShader(const char* name)
{
	auto i = _shaders.find(string(name));
	if (i != _shaders.end())
	{
		_currentShader = i->second;
		_currentShader->use();
		return _currentShader;
	}
	else
		return NULL;
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

Font* Engine::createFont(const string& name, const string& fontPath, int pixelSize)
{
	Font* newFont;

	// if a font already exists with the given name, delete it first
	auto i = _fonts.find(name);
	if (i != _fonts.end())
	{
		delete i->second;
		_fonts.erase(i);
	}

	// needed for the font texturing to work correctly
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	newFont = new Font();
	newFont->load(fontPath, pixelSize);
	newFont->setColor(0.929f, 0.5f, 0.455f);
	_fonts[name] = newFont;

	// if this is the only loaded font, bind it
	if (_fonts.size() == 1)
		_currentFont = newFont;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	return newFont;
}

Font* Engine::bindFont(const string& name)
{
	auto i = _fonts.find(name);
	if (i != _fonts.end())
	{
		_currentFont = i->second;
		return _currentFont;
	}
	else
		return NULL;
}

void Engine::setFontColor(float r, float g, float b)
{
	if (_currentFont != NULL)
		_currentFont->setColor(r, g, b);
}

void Engine::renderTextf(float x, float y, const string& format, ...)
{
	if (_currentFont != NULL)
	{
		char output[2001];
		va_list args;

		va_start(args, format);	
		vsnprintf_s(output, 2001, 2000, format.c_str(), args);
		va_end(args);

		bindShader("fontShader")->setMatrix4f("WVPMatrix", _orthoMatrix);		
		_currentFont->print(x, y, output);
		bindShader("mainShader");
	}
}

void Engine::destroyFont(const string& name)
{
	auto i = _fonts.find(name);
	if (i != _fonts.end())
	{
		delete i->second;
		_fonts.erase(i);
	}
}