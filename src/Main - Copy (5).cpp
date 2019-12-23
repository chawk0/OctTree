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
#include "Light.h"

using namespace std;

#define PRINT_GL_ERROR	printf("\tgl error: %d\n", glGetError());


//------------------------------------------------------------------------------

SDL_Window* window = NULL;
SDL_GLContext glContext = 0;
Uint8* keyStates = NULL;

Timer timer;
Camera camera;
ShaderProgram program;
Light directionalLight;
//Light spotLights[8];
Light spotLight;
int spotLightCount = 0;

//Matrix4 worldMatrix, viewMatrix, projectionMatrix, WVPMatrix;
//unsigned int worldMatrixLoc, WVPMatrixLoc;

SceneObject plane;
SceneObject cubes[3];
Texture cubeTexture;

//------------------------------------------------------------------------------

// hmm

//------------------------------------------------------------------------------

void initScene();
void initObjects();
void initShaders();
void initSDLGL();
void render(float dt);
void renderScene(float dt);
void renderScene2(float dt);
void updateMatrices(Matrix4& worldMatrix);
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
	Vector3 camPos;
	Matrix4 camView;
	Matrix4 lightWVP;
	static bool haveRead = false;
	static int frameCount = 0;
	unsigned char* buffer;
	unsigned int bufferLen;

	// shadow map pass
	/*
	spotLight.bindShadowMapForWriting();
	// only run the vertex shader, no fragment shader
	//glDrawBuffer(GL_NONE);
	glClearDepth(0.5f);
	glClear(GL_DEPTH_BUFFER_BIT);
	*/
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	//glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// save the current camera view and position, then set it to the spotlight	
	/*
	camPos = camera.getPosition();
	camera.setPosition(spotLight.getPos());
	camView = camera.getViewMatrix();
	camera.setViewMatrix(spotLight.getViewMatrix());
	camera.setProjection(spotLight.getProjectionMatrix());
	*/

	program.setUniform1i("useLighting", 0);
	program.setUniform1i("useTexturing", 0);
	renderScene2(dt);

	if (!haveRead && frameCount > 0)
	{
		//buffer = new unsigned char[512 * 512 * sizeof(float)];
		//memset(buffer, 0xCC, 512 * 512 * sizeof(float));
		bufferLen = 1440 * 900 * sizeof(float);
		buffer = new unsigned char[bufferLen];
		memset(buffer, 0xCC, bufferLen);
		glGetError();
		glReadPixels(0, 0, 1440, 900, GL_DEPTH_COMPONENT, GL_FLOAT, buffer);
		printf("readpixels: %d\n", glGetError());
		ofstream fileOut("depthbuffer.dat");
		fileOut.write((const char*)buffer, bufferLen);
		fileOut.close();
		delete [] buffer;
		haveRead = true;

		printf("predicted depth value: %.7f\n", camera.test());
	}
	/*
	// normal pass
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	//glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera.setProjection(1440, 900, 60.0f, 1.0f, 1000.0f);
	camera.setPosition(camPos);
	camera.setViewMatrix(camView);

	spotLight.bindShadowMapForReading(1);
	cubeTexture.bind(0);
	
	renderScene2(dt);

	if (!haveRead)
	{
		buffer = new unsigned char[1440 * 900 * 3 * sizeof(float)];
		memset(buffer, 0xCC, 1440 * 900 * 3 * sizeof(float));
		glGetError();
		glReadPixels(0, 0, 1440, 900, GL_RGB, GL_FLOAT, buffer);
		printf("readpixels: %d\n", glGetError());
		ofstream fileOut("colorbuffer.dat");
		fileOut.write((const char*)buffer, 1440 * 900 * 3 * sizeof(float));
		fileOut.close();
		delete [] buffer;
		haveRead = true;
	}
	*/
	
	// housekeeping?
	glBindVertexArray(0);
	//glUseProgram(0);

	frameCount++;
	
	SDL_GL_SwapWindow(window);
}

void renderScene(float dt)
{
	static MatrixStack t;
	static Matrix4 pvm;
	static Vector3 direction;
	static float angle = 0.0f;

	// ready the object-to-world matrix
	t.makeIdentity();

	// draw coordinate grid
	updateMatrices(t.top());
	program.setUniform1i("useLighting", 1);
	program.setUniform1i("useTexturing", 0);
	glVertexAttrib3f(1, 0.5f, 0.5f, 0.5f);
	plane.render();

	// draw cubes!
	program.setUniform1i("useTexturing", 1);
	t.push();
		t.translate(-3.0, 1.0f, 0.0f);
		for (int i = 0; i < 3; ++i)
		{
			updateMatrices(t.top());
			//cubeTexture.bind(0);
			// set the color to white
			glVertexAttrib3f(1, 1.0f, 1.0f, 1.0f);
			cubes[i].render();

			t.translate(3.0f, 0.0f, 0.0f);
		}
	t.pop();

	// draw the spotlight's model
	t.push();
		t.translate(spotLight.getPos());
		t.scale(0.1f, 0.1f, 0.1f);
		updateMatrices(t.top());

		program.setUniform1i("useLighting", 0);
		program.setUniform1i("useTexturing", 0);
		glVertexAttrib3f(1, spotLight.getColor().x, spotLight.getColor().y, spotLight.getColor().z);
		spotLight.render();
	t.pop();
	
	/*
	// render light models
	for (int i = 0; i < spotLightCount; ++i)
	{
		t.push();
			t.translate(spotLights[i].getPos());
			t.scale(0.1f, 0.1f, 0.1f);
			camera.setWorldMatrix(t.top());
			camera.apply();
			// manually fix vert attrib 1 to the camera's color
			glVertexAttrib3f(1, spotLights[i].getColor().x, spotLights[i].getColor().y, spotLights[i].getColor().z);
			spotLights[i].render();
		t.pop();
	}
	*/

	
	// yay sketch
	/*
	WVPMatrix = pvm * t.top();
	program.setMatrix4f(WVPMatrixLoc, WVPMatrix);
	Sketch::begin(GL_TRIANGLES, "wee");
		Sketch::color3f(1.0f, 1.0f, 0.0f);
		Sketch::vertex3f(-1.0f, -1.0f, 0.0f);
		Sketch::vertex3f( 1.0f, -1.0f, 0.0f);
		Sketch::vertex3f( 0.0f,  1.0f, 0.0f);
	Sketch::end();
	*/

	angle += (60.0f * dt);
	if (angle >= 360.0f)
		angle -= 360.0f;
}

void renderScene2(float dt)
{
	Matrix4 t;
	t.makeIdentity();
	updateMatrices(t);
	Sketch::begin(GL_TRIANGLE_STRIP, "fullscreenquad");
		Sketch::color3f(1.0f, 1.0f, 1.0f);
		Sketch::vertex3f(1.0f, -1.0f, 0.0f);
		Sketch::vertex3f(1.0f, 1.0f, 0.0f);
		Sketch::vertex3f(-1.0f, -1.0f, 0.0f);
		Sketch::vertex3f(-1.0f, 1.0f, 0.0f);
	Sketch::end();
}

void updateMatrices(Matrix4& worldMatrix)
{
	camera.setWorldMatrix(worldMatrix);
	camera.updateShader();
	//program.setMatrix4f("lightWVPMatrix", lightWVP * t.top());
	spotLight.setWorldMatrix(worldMatrix);
	spotLight.updateShader();
}

void initScene()
{
	printf("loading objects...\n");
	initObjects();
		
	printf("creating shaders and program...\n");
	initShaders();

	directionalLight.setType(Light::DIRECTIONAL);
	directionalLight.setColor(1.0f, 1.0f, 1.0f);
	directionalLight.setAmbient(0.010f);
	directionalLight.setDiffuse(0.0f);
	directionalLight.setDirection(Vector3(-5.0f, 10.0f, 5.0f).normalize());
	directionalLight.bind(&program, 0);

	// the initial spotlight
	spotLight.setType(Light::SPOT);
	spotLight.loadModel("./res/objects/pyramid_base.obj");
	spotLight.setColor(1.0f, 1.0f, 1.0f);
	spotLight.setAmbient(0.0f);
	spotLight.setDiffuse(1.0f);
	spotLight.setPos(0.0f, 0.0f, 5.0f);
	spotLight.setDirection(0.0f, 0.0f, -1.0f);
	//spotLight.setPos(-4.0f, 4.0f, 3.0f);
	//spotLight.lookAt(0.0f, 1.0f, 0.0f);
	
	//spotLight.setPos(0.0f, 3.0f, 0.0f);
	//spotLight.lookAt(0.0f, 3.0f, -1.0f);
	spotLight.setCutoff(cosf(30.0f * PI_OVER_180));
	spotLight.setAttenuation(0.0f, 0.0f, 1.0f / 20.0f);
	spotLight.createShadowMap(512, 512);

	spotLight.bind(&program, 0);
	spotLightCount++;
	
	program.setUniform1i("spotLightCount", spotLightCount);

	// init cam
	camera.setPosition(0.0f, 0.0f, 5.0f);
	camera.setProjection(1440, 900, 60.0f, 1.0f, 1000.0f);
	camera.bind(&program);
}

void initObjects()
{
	plane.load("plane", "./res/objects/plane.obj");
	plane.setVertexAttribIndex("pos", 0);
	plane.setVertexAttribIndex("color", 1);
	plane.setVertexAttribIndex("normal", 2);
	plane.build();
	
	char name[] = "cube ";
	for (int i = 0; i < 3; ++i)
	{
		name[4] = '0' + (char)i;
		//cube.load("cube", "./res/objects/indexed_cube.obj");
		cubes[i].load(name, "./res/objects/cube_n.obj");
		cubes[i].setVertexAttribIndex("pos", 0);
		//cubes[i].setVertexAttribIndex("color", 1);
		cubes[i].setVertexAttribIndex("normal", 2);
		cubes[i].setVertexAttribIndex("uv0", 3);
		cubes[i].build();
	}
	glGetError();
	cubeTexture.create("./res/textures/stone_base.tga");
}

void initShaders()
{
	//program.create("./res/shaders/BasicLighting.vp", "./res/shaders/BasicLighting.fp");
	program.create("./res/shaders/simple.vp", "./res/shaders/simple.fp");
	program.use();
	
	program.setUniform1i("baseMap", 0);
	program.setUniform1i("shadowMap", 1);
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

	time_t seed;
	time(&seed);
	srand((unsigned long)seed);
    	
	SDL_Event event;
    bool done = false;
	float mouseDX, mouseDY, dt;
	int frameCount = 0;
	char title[80];

	SDL_SetRelativeMouseMode(SDL_TRUE);

	printf("proj matrix?:");
	cout << Matrix4().makeProjection(1440.0f / 900.0f, 60.0f, 1.0f, 1000.0f);
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
						//cout << "cam pos: " << camera.getPosition() << endl;
						//cout << "cam forward: " << camera.getForward() << endl;

						/*
						if (spotLightCount < 8)
						{
							cout << "adding new spot light at " << camera.getPosition() << endl;
							cout << "\tdirection " << camera.getForward() << endl;

							//program.use();

							spotLights[spotLightCount].setType(Light::SPOT);
							spotLights[spotLightCount].loadModel("./res/objects/pyramid_base.obj");
							//spotLights[spotLightCount].setColor((float)(rand() % 256) / 255.0f, (float)(rand() % 256) / 255.0f, (float)(rand() % 256) / 255.0f);
							spotLights[spotLightCount].setColor(1.0f, 1.0f, 1.0f);
							spotLights[spotLightCount].setAmbient(0.0f);
							spotLights[spotLightCount].setDiffuse(1.0f);
							spotLights[spotLightCount].setPos(camera.getPosition());
							spotLights[spotLightCount].setDirection(camera.getForward());
							spotLights[spotLightCount].setCutoff(cosf(30.0f * PI_OVER_180));
							spotLights[spotLightCount].setAttenuation(0.0f, 0.0f, 1.0f / 30.0f);
							
							spotLights[spotLightCount].bind(&program, spotLightCount);
							spotLightCount++;
							program.setUniform1i("spotLightCount", spotLightCount);
							
							//glUseProgram(0);
						}
						else
							cout << "already at max spot lights!" << endl;
						*/
					}
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
