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

using namespace std;


//------------------------------------------------------------------------------

SDL_Window* window = NULL;
SDL_GLContext glContext = 0;
Uint8* keyStates = NULL;

Timer timer;

Camera camera;
Matrix4 projectionMatrix;

//Sketch sketch;
SceneObject cube2, grid, pyramid;
Texture baseMap, normalMap;

ShaderProgram program;
int projectionMatrixLoc, viewMatrixLoc, modelMatrixLoc;
int baseMapLoc;
int normalMapLoc;
int staticColorLoc;	// the static color for rendering arrays that lack color data
int lightPosLoc;

unsigned int fbo;
unsigned int fboTexture;

int currentFace = 0;

//------------------------------------------------------------------------------

// hmm

//------------------------------------------------------------------------------

void initScene();
void initObjects();
void initBuffers();
void initShaders();
void generateFaceTBN(float* v0, float* v1, float* v2);
unsigned int loadBuffer(GLenum target, GLvoid* data, GLsizeiptr size);
void initSDLGL();
void render(float dt);
void handleInput(float dt);

//------------------------------------------------------------------------------

// class funcs?

//------------------------------------------------------------------------------

void handleInput(float dt)
{
	float boost;
	float speed = 10.0f;

	keyStates = (Uint8*)SDL_GetKeyboardState(NULL);

	if (keyStates[SDL_SCANCODE_LCTRL] == 1 || keyStates[SDL_SCANCODE_RCTRL] == 1)
		boost = 10.0f;
	else
		boost = 1.0f;

	if (keyStates[SDL_SCANCODE_D] == 1)
		camera.translateX(speed * boost * dt);
	else if (keyStates[SDL_SCANCODE_A] == 1)
		camera.translateX(-speed * boost * dt);

	if (keyStates[SDL_SCANCODE_W] == 1)
		camera.translateY(speed * boost * dt);
	else if (keyStates[SDL_SCANCODE_S] == 1)
		camera.translateY(-speed * boost * dt);

	if (keyStates[SDL_SCANCODE_X] == 1)
		camera.translateZ(speed * boost * dt);
	else if (keyStates[SDL_SCANCODE_Z] == 1)
		camera.translateZ(-speed * boost * dt);

	if (keyStates[SDL_SCANCODE_E] == 1)
		camera.rotateZ(180.0f * dt);
	else if (keyStates[SDL_SCANCODE_Q] == 1)
		camera.rotateZ(-180.0f * dt);
}

void render(float dt)
{
	static MatrixStack t;
	static Matrix4 m;
	static Vector3 lightPos;
	static float angle = 0.0f;
	static int viewMatrixLoc;

	program.use();
	program.setMatrix4f("projectionMatrix", projectionMatrix);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// apply cam transform
	t.makeIdentity();
	t.apply(program.getUniform("viewMatrix"));

	// CUBE2, NEW AND IMPROVED
	t.push();
		t.translate(0.0f, 0.0f, -5.0f);
		t.rotateY(angle);
		t.apply(program.getUniform("modelMatrix"));
		
		cube2.render();
	t.pop();
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// apply cam transform
	t.loadMatrix(camera.getMatrix());
	t.translate(-camera.getPosition());
	t.apply(program.getUniform("viewMatrix"));

	// ready the model-to-world matrix
	t.makeIdentity();

	// CUBE2, NEW AND IMPROVED
	t.push();
		t.translate(-5.0f, 2.0f, -5.0f);
		t.rotateY(angle);
		t.apply(program.getUniform("modelMatrix"));
		
		cube2.render();
	t.pop();

	// a little light :3
	t.push();
		// orbit the pyramid
		t.translate(5.0f, 2.0f, -5.0f);
		t.rotateY(angle);
		//t.translate(0.0f, 0.5f, 0.5f);
		t.translate(0.0f, 0.0f, -2.5f);
		// teeny light
		t.scale(0.02f, 0.02f, 0.02f);
		// grab the light's position
		lightPos = Vector3(&(t.top().m[12]));
		t.apply(program.getUniform("modelMatrix"));

		program.setUniform3f("staticColor", 1.0f, 1.0f, 1.0f);
		pyramid.render();
		program.setUniform3f("staticColor", 0.0f, 0.0f, 0.0f);	// make sure to set it back to 0
	t.pop();
	

	// PYRAMID
	// setup tex units
	//baseMap.bind(0);
	//normalMap.bind(1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboTexture);
	
	// set baseMap sampler to tex unit 0, normalMap sampler to tex unit 1
	program.setUniform1i("baseMap", 0);
	program.setUniform1i("normalMap", 1);
	program.setUniform3f("lightPos", lightPos.x, lightPos.y, lightPos.z);

	t.push();
		t.translate(5.0f, 2.0f, -5.0f);
		t.apply(program.getUniform("modelMatrix"));

		pyramid.render();
	t.pop();

	// reset tex units, don't forget!
	//baseMap.unbind(0);
	//normalMap.unbind(1);
	glBindTexture(GL_TEXTURE_2D, 0);

	// GRID
	t.apply(program.getUniform("modelMatrix"));
	// draw the XZ grid and the 3 origin axes
	grid.render();
	
	
	// yay sketch
	t.apply(modelMatrixLoc);
	Sketch::begin(GL_TRIANGLES, "wee");
		Sketch::color3f(1.0f, 1.0f, 0.0f);
		Sketch::vertex3f(-1.0f, -1.0f, 0.0f);
		Sketch::vertex3f( 1.0f, -1.0f, 0.0f);
		Sketch::vertex3f( 0.0f,  1.0f, 0.0f);
	Sketch::end();

	// one more.  some GRASS
	t.push();
		t.translate(-5.0f, 0.0f, 0.0f);
		t.apply(modelMatrixLoc);	

		Sketch::begin(GL_TRIANGLES, "wee too");
			for (int i = 0; i < 16; i++)
			{
				float r = (float)(i % 4) / 3.0f;
				float g = 1.0f - (float)(i / 4) / 3.0f;
				float b = 0.0f;//0.5 * sinf(sqrt((float)((i % 4) * (i % 4) + (i / 4) * (i / 4)) / 4.0f / sqrt(2.0f) * 2.0f * PI));
				float dx = (float)(i % 4) - 1.5f;
				float dz = -(float)(i / 4) + 1.5f;

				Sketch::color3f(r, g, b);
				Sketch::vertex3f(-1.0f + dx, -1.0f, 0.0f + dz);
				Sketch::vertex3f( 1.0f + dx, -1.0f, 0.0f + dz);
				Sketch::vertex3f( 0.0f + dx,  1.0f, 0.0f + dz);
			}
		Sketch::end();
	t.pop();
	
	
	// housekeeping?
	glBindVertexArray(0);
	glUseProgram(0);
	
	angle += (120.0f * dt);
	if (angle >= 360.0f)
		angle -= 360.0f;

    SDL_GL_SwapWindow(window);
}

void initScene()
{
	printf("creating objects...\n");
	initObjects();
	printf("creating buffers...\n");
	initBuffers();
	
	printf("creating shaders and program...\n");
	initShaders();

	// init cam
	projectionMatrix.makeProjection(1440.0f / 900.0f, 60.0f, 1.0f, 1000.0f);
	camera.setPosition(0.0f, 3.0f, 8.0f);

	baseMap.create("./res/textures/stone_base.tga");
	normalMap.create("./res/textures/stone_normal.tga");

	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &fboTexture);

	glBindTexture(GL_TEXTURE_2D, fboTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1440, 900, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("error creating framebuffer, status: 0x%x\n", status);
}

void generateFaceTBN(float* pv0, float* pv1, float* pv2, float* pt, float* pb, float* pn)
{
	Vector3 v0, v1, v2, t, b, n;
	Vector3 A, B;
	Vector2 c0, c1, c2;
	float t1, t2, b1, b2, d;

	v0 = Vector3(pv0);
	v1 = Vector3(pv1);
	v2 = Vector3(pv2);

	c0 = Vector2(&pv0[3]);
	c1 = Vector2(&pv1[3]);
	c2 = Vector2(&pv2[3]);

	t1 = Vector2(c1 - c0).x;
	t2 = Vector2(c2 - c0).x;
	b1 = Vector2(c1 - c0).y;
	b2 = Vector2(c2 - c0).y;
	d = 1.0f / (t1 * b2 - t2 * b1);

	t = Vector3(v1 - v0) * d * b2 - Vector3(v2 - v0) * d * b1;
	t.normalize();
	b = Vector3(v1 - v0) * -d * t2 + Vector3(v2 - v0) * d * t1;
	b.normalize();

	/*A = v1 - v0;
	B = v2 - v0;
	n = A.cross(B);*/
	n = t.cross(b);

	pt[0] = t.x; pt[1] = t.y; pt[2] = t.z;
	pb[0] = b.x; pb[1] = b.y; pb[2] = b.z;
	pn[0] = n.x; pn[1] = n.y; pn[2] = n.z;
}

void initObjects()
{
	pyramid.load("pyramid", "./res/objects/pyramid.obj");
	pyramid.setVertexAttribIndex("verts", 0);
	pyramid.setVertexAttribIndex("uv0", 2);
	pyramid.setVertexAttribIndex("tangent", 3);
	pyramid.setVertexAttribIndex("binormal", 4);
	pyramid.setVertexAttribIndex("normal", 5);
	pyramid.build();

	grid.load("grid", "./res/objects/grid.obj");
	grid.setVertexAttribIndex("verts", 0);
	grid.setVertexAttribIndex("colors", 1);
	grid.build();
	
	cube2.load("cube2", "./res/objects/cube3.obj");
	cube2.setVertexAttribIndex("verts", 0);
	cube2.setVertexAttribIndex("colors", 1);
	cube2.build();
}

void initBuffers()
{
	// nope!
}

unsigned int loadBuffer(GLenum target, GLvoid* data, GLsizeiptr size)
{
	unsigned int buffer;

	glGenBuffers(1, &buffer);	
	glBindBuffer(target, buffer);
	glBufferData(target, size, data, GL_STATIC_DRAW);

	return buffer;
}

void initShaders()
{
	program.create("./res/shaders/temp.vp", "./res/shaders/temp.fp");
	program.use();

	printf("\tprojectionMatrix loc: %d\n", program.getUniform("projectionMatrix"));
	printf("\tviewMatrix loc: %d\n", program.getUniform("viewMatrix"));
	printf("\tmodelMatrix loc: %d\n", program.getUniform("modelMatrix"));

	printf("\tstaticColor loc: %d\n", program.getUniform("staticColor"));

	printf("\tbaseMap loc: %d\n", program.getUniform("baseMap"));
	printf("\tnormalMap loc: %d\n", program.getUniform("normalMap"));
	printf("\tlightPos loc: %d\n", program.getUniform("lightPos"));
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
	glGetError();	// glew fires a 1280. clear it
	
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
	//glDepthRange(0.0f, 1.0f);
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
	float mouseDX, mouseDY, dt;
	int frameCount = 0;
	char title[80];

	SDL_SetRelativeMouseMode(SDL_TRUE);

	printf("starting main loop...\n");
	timer.init();
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
					else if (event.key.keysym.sym == SDLK_SPACE)
					{
						printf("showing TBNs for face %d\n", currentFace);
						currentFace++;
						if (currentFace == 4)
							currentFace = 0;
					}
					//	cout << camera.getRight() << " " << camera.getUp() << " " << camera.getForward() << endl;
                    break;

				case SDL_MOUSEMOTION:
					mouseDX = (float)event.motion.xrel / 1439.0f;
					mouseDY = (float)event.motion.yrel / 899.0f;
					camera.rotateX(mouseDY * 135.0f);
					camera.rotateY(mouseDX * 135.0f);
            }
        }

		dt = timer.getElapsedSeconds();
		handleInput(dt);
		render(dt);
		//printf("\tgl error %d\n", glGetError());

		frameCount++;
		if (frameCount == 1000)
		{
			frameCount = 0;
			sprintf_s(title, 80, "jesus christ, fps: %d", (unsigned int)timer.getFPS(1000));
			SDL_SetWindowTitle(window, title);
		}
    }

	SDL_SetRelativeMouseMode(SDL_FALSE);

	SDL_Quit();
    return 0;
}
