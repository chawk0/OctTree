#include <windows.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>

#include <gl/glew.h>
#include <gl/gl.h>
//#include <gl/glu.h>
#include <sdl/SDL.h>

using namespace std;

//------------------------------------------------------------------------------

static SDL_Window* window = NULL;
static SDL_GLContext glContext = 0;

static unsigned int vertexArray, vertexBuffer, indexBuffer;

static const float vertexData[] = {-1.0f, -0.8660254f,
									1.0f, -0.8660254f,
									0.0f,  0.8660254f};

static const unsigned short indexData[] = {0, 1, 2};

static unsigned int program, vertexShader, fragmentShader;

static const char* vertexShaderSource = 
	"#version 330\n"
	"layout (location = 0) in vec2 inPos;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = vec4(inPos, -1.0, 1.0);\n"
	"}\n";

static const char* fragmentShaderSource =
	"#version 330\n"
	"out vec4 fragOut;\n"
	"void main()\n"
	"{\n"
	"   fragOut = vec4(0.0, 0.5, 0.0, 1.0);\n"
	"}\n";


//------------------------------------------------------------------------------

// nope

//------------------------------------------------------------------------------

void init();
void render();
void handleInput();

//------------------------------------------------------------------------------

// class funcs?

//------------------------------------------------------------------------------

void handleInput()
{
	// stuff involving keyStates
}

void render()
{
    glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(program);

	glBindVertexArray(vertexArray);
	
	int positionLocation = glGetAttribLocation(program, "inPos");
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	//glEnableVertexAttribArray(positionLocation);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(0);
	glUseProgram(0);
	
	
    SDL_GL_SwapWindow(window);
}

void init()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);

	cout << "creating buffers..." << endl;

	// create vertex and index buffers

	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

	cout << "creating shaders..." << endl;

	// create vertex and fragment shaders
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		int infoLogLength;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* infoLog = new char[infoLogLength];
		glGetShaderInfoLog(vertexShader, infoLogLength, NULL, infoLog);

		cout << "error compiling vertex shader:" << endl << infoLog << endl;
		delete[] infoLog;
	}

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		int infoLogLength;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* infoLog = new char[infoLogLength];
		glGetShaderInfoLog(fragmentShader, infoLogLength, NULL, infoLog);

		cout << "error compiling fragment shader:" << endl << infoLog << endl;
		delete[] infoLog;
	}

	cout << "creating program..." << endl;

	// create program
	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		int infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* infoLog = new char[infoLogLength];
		glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
		
		cout << "error linking program:" << endl << infoLog << endl;
		delete[] infoLog;
	}
	
	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
}

/*
int main(int argCount, char *argValues[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	//SDL_Quit();

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600
	Uint32 flags = SDL_HWSURFACE | SDL_OPENGL | SDL_DOUBLEBUF;// | SDL_FULLSCREEN;
	screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, flags);
	
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClearColor(0.0, 0.0, 0.5, 1.0);
    
	unsigned int vbo;
	float vboData[] = {-1.0f, -0.8660254f, 1.0f, -0.8660254f, 0.0f,  0.8660254f};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vboData), vboData, GL_STATIC_DRAW);


	SDL_Event event;
    bool done = false;
    Uint32 frameCount = 0;
    Uint32 lastTime = SDL_GetTicks(), temp, frameTime;
    float fps = 0.0f;
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

                case SDL_KEYUP:
                {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                    }
                    break;
                }
            }
        }

		keyStates = SDL_GetKeyState(NULL);
		handleInput();
        loop();
        frameCount++;
        
		
        if (frameCount == 60)
        {
            temp = SDL_GetTicks();
            frameTime = temp - lastTime;
            fps = 60.0f * 1000.0f / (float)frameTime;
            lastTime = temp;
            frameCount = 0;
            sprintf_s(title, 80, "FPS: %.1f, frametime: %.3f", fps, (float)frameTime / 60.0f);
            SDL_WM_SetCaption(title, NULL);
        }
		
    }

	SDL_Quit();
    return 0;
}
*/
int main(int argCount, char *argValues[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
        MessageBox(NULL, L"blarg", L"error", MB_OK);
 
    /* Request opengl 3.2 context.
     * SDL doesn't have the ability to choose which profile at this time of writing,
     * but it should default to the core profile */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
 
    /* Turn on double buffering with a 24bit Z buffer.
     * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
 
    /* Create our window centered at 512x512 resolution */
    window = SDL_CreateWindow("jesus christ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window) /* Die if creation failed */
        MessageBox(NULL, L"blarg", L"error", MB_OK);
 
    /* Create our opengl context and attach it to our window */
    glContext = SDL_GL_CreateContext(window);
	if (glContext == NULL)
		MessageBoxA(NULL, SDL_GetError(), "SDL Context Error", MB_OK);

	glewExperimental = GL_TRUE; 
	glewInit();
	init();
    	
	/*typedef GLuint (WINAPI *PFNGLCREATEPROGRAM)(void);
	typedef void (WINAPI *PFNGLGENBUFFERS)(GLsizei n, GLuint* buffers);
	typedef void (WINAPI *PFNGLBINDBUFFER)(unsigned int target, GLuint buffer);
	typedef void (WINAPI *PFNGLBUFFERDATA)(unsigned int target, int size, const GLvoid* data, unsigned int usage);
	

	//#define GL_ARRAY_BUFFER 0x8892
	//#define GL_STATIC_DRAW 0x88E4

	PFNGLCREATEPROGRAM glCreateProgram2 = NULL;
	PFNGLGENBUFFERS glGenBuffers = NULL;
	PFNGLBINDBUFFER glBindBuffer = NULL;
	PFNGLBUFFERDATA glBufferData = NULL;
	
	//glCreateProgram2 = (PFNGLCREATEPROGRAM)wglGetProcAddress("glCreateProgram");
	// BREAK
	//glCreateProgram2();
	//glGenBuffers = (PFNGLGENBUFFERS)wglGetProcAddress("glGenBuffers");
	//glBindBuffer = (PFNGLBINDBUFFER)wglGetProcAddress("glBindBuffer");
	//glBufferData = (PFNGLBUFFERDATA)wglGetProcAddress("glBufferData");*/
	
	/*
	unsigned int vbo;
	float vboData[] = {-1.0f, -0.8660254f, 1.0f, -0.8660254f, 0.0f,  0.8660254f};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vboData), vboData, GL_STATIC_DRAW);
		
	static wchar_t text[100];
	_itow_s(vbo, text, 100, 10);
	MessageBox(NULL, text, text, MB_OK);
		*/

	/* This makes our buffer swap syncronized with the monitor's vertical refresh */
	SDL_Event event;
    bool done = false;
    Uint32 frameCount = 0;
    Uint32 lastTime = SDL_GetTicks(), temp, frameTime;
    float fps = 0.0f;
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

                case SDL_KEYUP:
                {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                    }
                    break;
                }
            }
        }

		//keyStates = SDL_GetKeyState(NULL);
		//handleInput();
        render();
        //frameCount++;
        
		/*
        if (frameCount == 60)
        {
            temp = SDL_GetTicks();
            frameTime = temp - lastTime;
            fps = 60.0f * 1000.0f / (float)frameTime;
            lastTime = temp;
            frameCount = 0;
            sprintf_s(title, 80, "FPS: %.1f, frametime: %.3f", fps, (float)frameTime / 60.0f);
            SDL_WM_SetCaption(title, NULL);
        }*/
		
    }

	SDL_Quit();
    return 0;
}
