/*
	OpenGL 3.3 + SDL, in Windows, with Visual Studio, in C++

	CHRIST

	-chawk Jul 2013
*/

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
#include <sdl/SDL_image.h>

/*
#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "MatrixStack.h"
#include "Texture.h"
#include "Camera.h"
#include "Sketch.h"
#include "ShaderProgram.h"
#include "SceneObject.h"
#include "Timer.h"
#include "Light.h"
*/

using namespace std;

//#define PRINT_GL_ERROR	printf("\tgl error: %d\n", glGetError());


//------------------------------------------------------------------------------

SDL_Window* window = NULL;
SDL_GLContext glContext = 0;
Uint8* keyStates = NULL;

unsigned int vao, vbo;
unsigned int texture;
unsigned int shader;

const float vertexData[] = {1.0f, -1.0f, 0.0f,
							1.0f, 1.0f, 0.0f,
							-1.0f, -1.0f, 0.0f,
							-1.0f, -1.0f, 0.0f,
							1.0f, 1.0f, 0.0f,
							-1.0f, 1.0f, 0.0f,
							1.0f, 0.0f,
							1.0f, 1.0f,
							0.0f, 0.0f,
							0.0f, 0.0f,
							1.0f, 1.0f,
							0.0f, 1.0f};

const char* vertexShaderSource = 
	"#version 330\n"
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 8) in vec2 uv;\n"
	"smooth out vec2 uv0;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = vec4(position, 1.0);\n"
	"   uv0 = uv;\n"
	"}\n";

const char* fragmentShaderSource =
	"#version 330\n"
	"uniform sampler2D tex;\n"
	"in vec2 uv0;\n"
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"   color = texture(tex, uv0);\n"
	"}\n";

//------------------------------------------------------------------------------

// hmm

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

// class funcs?

//------------------------------------------------------------------------------

void renderScene()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
	
	SDL_GL_SwapWindow(window);
}

void initScene()
{
	unsigned int vertexShader, fragmentShader;

	glViewport(0, 0, 1440, 900);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(sizeof(float) * 18));

	glBindVertexArray(0);

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

		MessageBox(NULL, L"error compiling vertex shader :(", L"wow such error", MB_OK | MB_ICONERROR);
		delete[] infoLog;
		return;
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

		MessageBox(NULL, L"error compiling fragment shader :(", L"wow such error", MB_OK | MB_ICONERROR);
		delete[] infoLog;
		return;
	}

	// create program
	shader = glCreateProgram();
	glAttachShader(shader, vertexShader);
	glAttachShader(shader, fragmentShader);
	glLinkProgram(shader);

	glGetProgramiv(shader, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		int infoLogLength;
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* infoLog = new char[infoLogLength];
		glGetProgramInfoLog(shader, infoLogLength, NULL, infoLog);
		
		MessageBox(NULL, L"error linking program :(", L"wow such error", MB_OK | MB_ICONERROR);
		delete[] infoLog;
		return;
	}

	glDetachShader(shader, vertexShader);
	glDetachShader(shader, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUniform1i(glGetUniformLocation(shader, "tex"), 0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	SDL_Surface* s;
    SDL_RWops* rwops;
    
    rwops = SDL_RWFromFile("./res/textures/stone_base.tga", "rb");
    if (!rwops)
		printf("[ERROR]: Texture::loadToBuffer(): failed to get RWops for texture file.");
	else
	{
		s = IMG_LoadTGA_RW(rwops);

		if (s->format->BitsPerPixel == 24)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s->w, s->h, 0, GL_BGR, GL_UNSIGNED_BYTE, s->pixels);
		else if (s->format->BitsPerPixel == 32)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s->w, s->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, s->pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

/*
	unsigned char* textureData = new unsigned char[512 * 512 * 3];
	unsigned char r, g, b;
	int index;

	for (int y = 0; y < 512; ++y)
	{
		for (int x = 0; x < 512; ++x)
		{
			index = (x + 512 * y) * 3;

			if (x < 256 && y < 256)
			{
				r = 255;
				g = 0;
				b = 0;
			}
			else if (x >= 256 && y < 256)
			{
				r = 0;
				g = 255;
				b = 0;
			}
			else if (x < 256 && y >= 256)
			{
				r = 0;
				g = 0;
				b = 255;
			}
			else
			{
				r = 255;
				g = 255;
				b = 255;
			}
			
			textureData[index + 0] = r;
			textureData[index + 1] = g;
			textureData[index + 2] = b;
		}
	}
*/

	glBindTexture(GL_TEXTURE_2D, 0);
}

void initSDLGL()
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
    window = SDL_CreateWindow("jesus christ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1440, 900, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window) /* Die if creation failed */
	{
		printf("error: failed to create window\n\t%s\n", SDL_GetError());
		exit(1);
	}
 
    /* Create our opengl context and attach it to our window */
    glContext = SDL_GL_CreateContext(window);
	if (glContext == NULL)
	{
		printf("error: failed to create GL context\n\t%s\n", SDL_GetError());
		exit(1);
	}

	SDL_GL_SetSwapInterval(0);

	glewExperimental = GL_TRUE; 
	glewInit();
}

void cleanup()
{
	glDeleteTextures(1, &texture);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteShader(shader);
}

int main(int argCount, char *argValues[])
{
    initSDLGL();
	initScene();

	time_t seed;
	time(&seed);
	srand((unsigned long)seed);
    	
	SDL_Event event;
    bool done = false;

	//SDL_SetRelativeMouseMode(SDL_TRUE);

    while (!done)
    {
        while (SDL_PollEvent(&event) == 1)
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    done = true;
                    break;
            }
        }

		renderScene();
    }

	//SDL_SetRelativeMouseMode(SDL_FALSE);

	cleanup();
	SDL_Quit();
    return 0;
}
