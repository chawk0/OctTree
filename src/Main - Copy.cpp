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

#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "MatrixStack.h"

using namespace std;

//------------------------------------------------------------------------------

SDL_Window* window = NULL;
SDL_GLContext glContext = 0;

Matrix4 projectionMatrix;
int projectionMatrixLoc, modelViewMatrixLoc;

/*
unsigned int cubeVAO = 0, pyramidVAO = 0, gridVAO = 0;
unsigned int cubeVBO = 0, pyramidVBO = 0, gridVBO = 0;
unsigned int cubeIBO = 0, pyramidIBO = 0;
*/

#define CUBE	0
#define PYRAMID	1
#define GRID	2
#define GRID_LENGTH	40

unsigned int VAOs[3];
unsigned int VBOs[3];
unsigned int IBOs[2];

unsigned int shaderProgram;

//------------------------------------------------------------------------------

// hmm

//------------------------------------------------------------------------------

void initScene();
void initObjects();
void initShaders();
unsigned int loadBuffer(GLenum target, GLvoid* data, GLsizeiptr size);
unsigned int loadShader(const char* fileName, GLenum shaderType);
unsigned int createProgram(unsigned int vs, unsigned int fs);
void initSDLGL();
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
	static MatrixStack t;
	static float angle = 0.0f;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, projectionMatrix.m);

	// apply cam transform
	t.makeIdentity();
	t.translate(0.0f, -1.0f, 0.0f);

	// CUBE
	t.push();		
		t.rotateY(angle);
		t.setWorldPos(0.0f, 2.0f, -5.0f);
		t.apply(modelViewMatrixLoc);
		
		// the one VAO saves the buffer and vertex attrib pointer bindings in one quick call, neat.
		glBindVertexArray(VAOs[CUBE]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
	t.pop();

	// GRID
	t.apply(modelViewMatrixLoc);
	glBindVertexArray(VAOs[GRID]);
	// draw the XZ grid and the 3 origin axes
	glDrawArrays(GL_LINES, 0, 2 * (2 * (GRID_LENGTH + 1) + 3));

	glBindVertexArray(0);
	glUseProgram(0);

	angle += 3.0f;
	if (angle >= 360.0f)
		angle -= 360.0f;

    SDL_GL_SwapWindow(window);
}

void initScene()
{

	printf("creating objects and buffers...\n");
	initObjects();

	printf("creating shaders and program...\n");
	initShaders();

	//projectionMatrix.makeIdentity();
	projectionMatrix.makeProjection(800.0f/600.0f, 60.0f, 1.0f, 1000.0f);
}

void initObjects()
{
	// interleaved cube vertex and color data
	// x, y, z, r, g, b
	float cubeData[] = {-1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
						 1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
						 1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
						-1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f,

						-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
						 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
						-1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f};
	// triangle index data
	unsigned short cubeIndices[] = {0, 1, 2, 2, 3, 0,	// front
									3, 2, 6, 6, 7, 3,	// top
									1, 5, 6, 6, 2, 1,	// right
									4, 0, 3, 3, 7, 4,	// left
									5, 4, 7, 7, 6, 5,	// back
									4, 5, 1, 1, 0, 4};	// bottom
	
	float pyramidData[] = {0.0f};
	unsigned short pyramidIndices[] = {0};

	// grid_length of N means a grid from -N to N, which is (grid_length + 1) lines
	// line count times 2 for X and Z directions, add 3 for the origin axes
	// then times 2 vertices per line, times 6 floats per vertex (x, y, z, r, g, b)
	float gridData[(2 * (GRID_LENGTH + 1) + 3) * 2 * 6];

	// initialize the grid data
	float halfGrid = (float)GRID_LENGTH / 2.0f;
	int zLinesOffset = (GRID_LENGTH + 1) * 12;
	for (int i = 0; i < (GRID_LENGTH + 1); i++)
	{
		// X lines
		gridData[i * 12 + 0] = (float)i - halfGrid;
		gridData[i * 12 + 1] = 0.0f;
		gridData[i * 12 + 2] = halfGrid;
		gridData[i * 12 + 3] = 0.5f;
		gridData[i * 12 + 4] = 0.5f;
		gridData[i * 12 + 5] = 0.5f;

		gridData[i * 12 + 6] = (float)i - halfGrid;
		gridData[i * 12 + 7] = 0.0f;
		gridData[i * 12 + 8] = -halfGrid;
		gridData[i * 12 + 9] = 0.5f;
		gridData[i * 12 + 10] = 0.5f;
		gridData[i * 12 + 11] = 0.5f;
		
		// Z lines
		gridData[zLinesOffset + i * 12 + 0] = halfGrid;
		gridData[zLinesOffset + i * 12 + 1] = 0.0f;
		gridData[zLinesOffset + i * 12 + 2] = (float)i - halfGrid;
		gridData[zLinesOffset + i * 12 + 3] = 0.5f;
		gridData[zLinesOffset + i * 12 + 4] = 0.5f;
		gridData[zLinesOffset + i * 12 + 5] = 0.5f;

		gridData[zLinesOffset + i * 12 + 6] = -halfGrid;
		gridData[zLinesOffset + i * 12 + 7] = 0.0f;
		gridData[zLinesOffset + i * 12 + 8] = (float)i - halfGrid;
		gridData[zLinesOffset + i * 12 + 9] = 0.5f;
		gridData[zLinesOffset + i * 12 + 10] = 0.5f;
		gridData[zLinesOffset + i * 12 + 11] = 0.5f;
	}

	int originAxesOffset = 2 * (GRID_LENGTH + 1) * 12;
	//red X unit vector
	gridData[originAxesOffset + 0 * 12 + 0] = 0.0f;
	gridData[originAxesOffset + 0 * 12 + 1] = 0.0f;
	gridData[originAxesOffset + 0 * 12 + 2] = 0.0f;
	gridData[originAxesOffset + 0 * 12 + 3] = 1.0f;
	gridData[originAxesOffset + 0 * 12 + 4] = 0.0f;
	gridData[originAxesOffset + 0 * 12 + 5] = 0.0f;

	gridData[originAxesOffset + 0 * 12 + 6] = 1.0f;
	gridData[originAxesOffset + 0 * 12 + 7] = 0.0f;
	gridData[originAxesOffset + 0 * 12 + 8] = 0.0f;
	gridData[originAxesOffset + 0 * 12 + 9] = 1.0f;
	gridData[originAxesOffset + 0 * 12 + 10] = 0.0f;
	gridData[originAxesOffset + 0 * 12 + 11] = 0.0f;

	// green Y unit vector
	gridData[originAxesOffset + 1 * 12 + 0] = 0.0f;
	gridData[originAxesOffset + 1 * 12 + 1] = 0.0f;
	gridData[originAxesOffset + 1 * 12 + 2] = 0.0f;
	gridData[originAxesOffset + 1 * 12 + 3] = 0.0f;
	gridData[originAxesOffset + 1 * 12 + 4] = 1.0f;
	gridData[originAxesOffset + 1 * 12 + 5] = 0.0f;

	gridData[originAxesOffset + 1 * 12 + 6] = 0.0f;
	gridData[originAxesOffset + 1 * 12 + 7] = 1.0f;
	gridData[originAxesOffset + 1 * 12 + 8] = 0.0f;
	gridData[originAxesOffset + 1 * 12 + 9] = 0.0f;
	gridData[originAxesOffset + 1 * 12 + 10] = 1.0f;
	gridData[originAxesOffset + 1 * 12 + 11] = 0.0f;

	// blue Z unit vector
	gridData[originAxesOffset + 2 * 12 + 0] = 0.0f;
	gridData[originAxesOffset + 2 * 12 + 1] = 0.0f;
	gridData[originAxesOffset + 2 * 12 + 2] = 0.0f;
	gridData[originAxesOffset + 2 * 12 + 3] = 0.0f;
	gridData[originAxesOffset + 2 * 12 + 4] = 0.0f;
	gridData[originAxesOffset + 2 * 12 + 5] = 1.0f;

	gridData[originAxesOffset + 2 * 12 + 6] = 0.0f;
	gridData[originAxesOffset + 2 * 12 + 7] = 0.0f;
	gridData[originAxesOffset + 2 * 12 + 8] = 1.0f;
	gridData[originAxesOffset + 2 * 12 + 9] = 0.0f;
	gridData[originAxesOffset + 2 * 12 + 10] = 0.0f;
	gridData[originAxesOffset + 2 * 12 + 11] = 1.0f;



	// 1 VAO per object
	glGenVertexArrays(3, &VAOs[0]);
		
	// bind the VAO, then setup buffer binding and vertex attributes
	glBindVertexArray(VAOs[CUBE]);

	// enable generic vertex attributes that seem to pass to the shader in the layout section
	// 0 = vertex, 1 = color
	// these both reference the same VBO, just interleaved
	VBOs[CUBE] = loadBuffer(GL_ARRAY_BUFFER, cubeData, sizeof(cubeData));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0); 
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3));

	// defining how this array is used is done via the args to glDrawElements
	IBOs[CUBE] = loadBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndices, sizeof(cubeIndices));

	// now the other 2 objects
	//glBindVertexArray(VAOs[PYRAMID]);

	
	glBindVertexArray(VAOs[GRID]);

	VBOs[GRID] = loadBuffer(GL_ARRAY_BUFFER, gridData, sizeof(gridData));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0); 
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3));
}

unsigned int loadBuffer(GLenum target, GLvoid* data, GLsizeiptr size)
{
	unsigned int buffer;

	glGenBuffers(1, &buffer);
	glBindBuffer(target, buffer);
	glBufferData(target, size, data, GL_STATIC_DRAW);

	return buffer;
}

/*
void initBuffers()
{
	//printf("sizeof(cubeData): %d, sizeof(cubeIndices): %d\n", sizeof(cubeData), sizeof(cubeIndices));

	// generate 1 VAO per "object"
	glGenVertexArrays(1, &cubeVAO);
	//glGenVertexArrays(1, &pyramidVAO);
	//glGenVertexArrays(1, &gridVAO);

	// the cube
	glBindVertexArray(cubeVAO);

	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeData), cubeData, GL_STATIC_DRAW);

	// enable generic vertex attributes that seem to pass to the shader in the layout section
	// 0 = vertex, 1 = color
	// these both reference the same VBO, just interleaved
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0); 
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3));
	
	glGenBuffers(1, &cubeIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
	

	/*
	// the pyramid
	glBindVertexArray(pyramidVAO);

	glGenBuffers(1, &pyramidVBO);
	glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidData), pyramidData, GL_STATIC_DRAW);
	
	glGenBuffers(1, &pyramidIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);
		
	// the coordinate grid, without index buffers, just straight vertex data
	glBindVertexArray(gridVAO);

	glGenBuffers(1, &gridVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gridData), gridData, GL_STATIC_DRAW);
	
}*/

void initShaders()
{
	unsigned int vs, fs;
	vs = loadShader("temp.vs", GL_VERTEX_SHADER);
	fs = loadShader("temp.fs", GL_FRAGMENT_SHADER);
	shaderProgram = createProgram(vs, fs);

	glDetachShader(shaderProgram, vs);
	glDetachShader(shaderProgram, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);

	projectionMatrixLoc = glGetUniformLocation(shaderProgram, "projectionMatrix");
	modelViewMatrixLoc = glGetUniformLocation(shaderProgram, "modelViewMatrix");
}

unsigned int loadShader(const char* fileName, GLenum shaderType)
{
	unsigned int shader;

	if (shaderType == GL_VERTEX_SHADER)
		shader = glCreateShader(GL_VERTEX_SHADER);
	else if (shaderType == GL_FRAGMENT_SHADER)
		shader = glCreateShader(GL_FRAGMENT_SHADER);
	else
	{
		printf("error loading shader: invalid shader type\n");
		return 0;
	}

	fstream fileIn;
	string source;
	char buf[200];
	char* infoLog;
	char* sourcePtr;
	int status, infoLogLength;

	fileIn.open(fileName, ios:: in);
	if (fileIn)
	{
		while (!fileIn.eof())
		{
			fileIn.getline(buf, 200);
			source = source + string(buf) + "\n";
		}

		//cout << "\n\n" << source << "\n\n";

		sourcePtr = (char*)source.c_str();
		glShaderSource(shader, 1, (const GLchar**)&sourcePtr, NULL);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

			infoLog = new char[infoLogLength];
			glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);

			printf("error compiling vertex shader:\n%s\n", infoLog);
			delete[] infoLog;
			glDeleteShader(shader);
			return 0;
		}

		return shader;		
	}
	else
	{
		printf("error loading shader file: %s\n", fileName);
		return 0;
	}
}

unsigned int createProgram(unsigned int vs, unsigned int fs)
{
	unsigned int program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	int status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		int infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* infoLog = new char[infoLogLength];
		glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
		
		printf("error linking shader program:\n%s\n", infoLog);
		delete[] infoLog;
		glDeleteProgram(program);
		return 0;
	}
	else
		return program;
}

void initSDLGL()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
	{
		printf("error: failed to init SDL\n\t%s\n\n", SDL_GetError());
		exit(1);
	}
 
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
        800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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

	glewExperimental = GL_TRUE; 
	glewInit();

	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
}

int main(int argCount, char *argValues[])
{
    initSDLGL();
	initScene();
    	
	SDL_Event event;
    bool done = false;

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

		handleInput();
        render();
    }

	SDL_Quit();
    return 0;
}


