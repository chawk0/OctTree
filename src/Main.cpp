/*
	Chawk Fortress? yeah right

	-chawk Jan 2014
*/

#include <windows.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>

#include <gl/glew.h>
#include <gl/gl.h>
//#include <gl/glu.h>
#include <sdl/SDL.h>
#include <sdl/SDL_image.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "ChawkFortress.h"

#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Plane.h"
#include "Frustum.h"
#include "AABox.h"

#include "Engine.h"
#include "Mesh.h"
#include "Sketch.h"
#include "Font.h"
#include "Light.h"

using namespace std;

#define PRINT_GL_ERROR	printf("\tgl error: %d\n", glGetError());


//------------------------------------------------------------------------------

Engine engine;
Mesh grid, cube, cube2;
Vector3 cubePos;
Octree::Node* currentNode, *lastNode, *highlightNode;
uint renderMode, maxDepth;
//AABox box;

uint fontTexture;
Matrix4 testMatrix;
RenderData fontTest;
//FT_Library freeTypeInstance;
//FT_Face testFontFace;
Font* testFont = NULL;
Texture testTexture;
Light light1;

//------------------------------------------------------------------------------

// hmm

//------------------------------------------------------------------------------

void init();
void initShaders();
void initObjects();
void initLights();
void render(double dt);
void handleInput(double dt);
void cleanup();

//------------------------------------------------------------------------------

void handleInput(double dt)
{
	static Uint8 keyState[SDL_NUM_SCANCODES];
	static Uint8 keyDown[SDL_NUM_SCANCODES];
	static Uint8 keyUp[SDL_NUM_SCANCODES];
	static bool firstPass = true;
	Camera* cam;
	float boost;
	float speed = 10.0f;

	cam = engine.getCamera();

	if (firstPass)
	{
		// on the first pass of this function, copy the initial state and set keytaps to 0
		memcpy(keyState, SDL_GetKeyboardState(NULL), sizeof(Uint8) * SDL_NUM_SCANCODES);
		memset(keyDown, 0, sizeof(Uint8) * SDL_NUM_SCANCODES);
		memset(keyUp, 0, sizeof(Uint8) * SDL_NUM_SCANCODES);
		firstPass = false;
	}
	else
	{
		// null out the key down and up arrays.  if a key is found to change state, then
		// only for this frame will the relevant array be equal to 1 per below's logic.
		memset(keyDown, 0, sizeof(Uint8) * SDL_NUM_SCANCODES);
		memset(keyUp, 0, sizeof(Uint8) * SDL_NUM_SCANCODES);

		// then, compare the state of each key to the last frame.  if a key was either
		// pressed down or released, this change of state is recorded in the respective array
		Uint8* newKeyState = (Uint8*)SDL_GetKeyboardState(NULL);
		for (unsigned int i = 0; i < SDL_NUM_SCANCODES; ++i)
		{
			// if the keys differ, the state has changed
			if (newKeyState[i] != keyState[i])
			{
				// key down event
				if (newKeyState[i] == 1)
				{
					keyDown[i] = 1;
					keyUp[i] = 0;
				}
				// key up event
				else
				{
					keyDown[i] = 0;
					keyUp[i] = 1;
				}
				
				// then copy the new state to make things current
				keyState[i] = newKeyState[i];
			}
		}
	}

	// holding ctrl makes the cam move 10x as fast
	if (keyState[SDL_SCANCODE_LCTRL] == 1 || keyState[SDL_SCANCODE_RCTRL] == 1)
		boost = 10.0f;
	else
		boost = 1.0f;

	// toggle varies render modes
	if (keyDown[SDL_SCANCODE_SPACE] == 1)
	{
		renderMode++;
		if (renderMode == 7)
			renderMode = 1;
		switch (renderMode)
		{
			case 1:
				cout << "using world::render" << endl;
				break;
			case 2:
				cout << "using octree::render3" << endl;
				break;
			case 3:
				cout << "using octree::render4" << endl;
				break;
			case 4:
				cout << "using octree::renderWireframes" << endl;
				break;
			case 5:
				cout << "using octree::renderNodeTest" << endl;
				break;
			case 6:
				cout << "using octree::renderNodeTest2" << endl;
				break;
		};
	}

	// WSAD control camera x and y.  ZX control z.  QE control roll
	if (keyState[SDL_SCANCODE_D] == 1)
		cam->translateX(speed * boost * static_cast<float>(dt));
	else if (keyState[SDL_SCANCODE_A] == 1)
		cam->translateX(-speed * boost * static_cast<float>(dt));

	if (keyState[SDL_SCANCODE_W] == 1)
		cam->translateY(speed * boost * static_cast<float>(dt));
	else if (keyState[SDL_SCANCODE_S] == 1)
		cam->translateY(-speed * boost * static_cast<float>(dt));

	if (keyState[SDL_SCANCODE_X] == 1)
		cam->translateZ(speed * boost * static_cast<float>(dt));
	else if (keyState[SDL_SCANCODE_Z] == 1)
		cam->translateZ(-speed * boost * static_cast<float>(dt));

	if (keyState[SDL_SCANCODE_E] == 1)
		cam->rotateZ(180.0f * static_cast<float>(dt));
	else if (keyState[SDL_SCANCODE_Q] == 1)
		cam->rotateZ(-180.0f * static_cast<float>(dt));

	// only valid for renderNodeTest
	if (renderMode == 5)
	{
		// test object moves around via arrow keys and pageup/pagedown
		if (keyState[SDL_SCANCODE_RIGHT] == 1)
			cubePos.x += (speed * boost * static_cast<float>(dt));
		else if (keyState[SDL_SCANCODE_LEFT] == 1)
			cubePos.x -= (speed * boost * static_cast<float>(dt));

		if (keyState[SDL_SCANCODE_UP] == 1)
			cubePos.z -= (speed * boost * static_cast<float>(dt));
		else if (keyState[SDL_SCANCODE_DOWN] == 1)
			cubePos.z += (speed * boost * static_cast<float>(dt));

		if (keyState[SDL_SCANCODE_PAGEUP] == 1)
			cubePos.y += (speed * boost * static_cast<float>(dt));
		else if (keyState[SDL_SCANCODE_PAGEDOWN] == 1)
			cubePos.y -= (speed * boost * static_cast<float>(dt));

		// change the max depth to which _findNode searches
		if (keyDown[SDL_SCANCODE_KP_PLUS] == 1)
		{
			if (maxDepth < engine._octree._treeDepth)
			{
				maxDepth++;
				cout << "max depth " << maxDepth << endl;
			}
		}
		else if (keyDown[SDL_SCANCODE_KP_MINUS] == 1)
		{
			if (maxDepth > 0)
			{
				maxDepth--;
				cout << "max depth " << maxDepth << endl;
			}
		}
	}
	else if (renderMode == 6)
	{
		bool nodeHasChanged = false;

		// +X
		if (keyDown[SDL_SCANCODE_RIGHT] == 1)
		{
			if (currentNode->neighbors[0] != NULL)
			{
				currentNode = currentNode->neighbors[0];
				nodeHasChanged = true;
			}
		}
		// -X
		else if (keyDown[SDL_SCANCODE_LEFT] == 1)
		{
			if (currentNode->neighbors[1] != NULL)
			{
				currentNode = currentNode->neighbors[1];
				nodeHasChanged = true;
			}
		}
		// -Z
		else if (keyDown[SDL_SCANCODE_UP] == 1)
		{
			if (currentNode->neighbors[5] != NULL)
			{
				currentNode = currentNode->neighbors[5];
				nodeHasChanged = true;
			}
		}
		// +Z
		else if (keyDown[SDL_SCANCODE_DOWN] == 1)
		{
			if (currentNode->neighbors[4] != NULL)
			{
				currentNode = currentNode->neighbors[4];
				nodeHasChanged = true;
			}
		}
		// +Y
		else if (keyDown[SDL_SCANCODE_PAGEUP] == 1)
		{
			if (currentNode->neighbors[2] != NULL)
			{
				currentNode = currentNode->neighbors[2];
				nodeHasChanged = true;
			}
		}
		// -Y
		else if (keyDown[SDL_SCANCODE_PAGEDOWN] == 1)
		{
			if (currentNode->neighbors[3] != NULL)
			{
				currentNode = currentNode->neighbors[3];
				nodeHasChanged = true;
			}
		}
		// numpad0 switches to the current node's parent
		else if (keyDown[SDL_SCANCODE_KP_0] == 1)
		{
			if (currentNode->parent != NULL)
			{
				currentNode = currentNode->parent;
				nodeHasChanged = true;
			}
		}
		// else, check for numpad1-8 to switch children
		else if (!currentNode->isLeaf)
		{
			for (uint i = 0; i < 8; ++i)
			{
				if (keyDown[SDL_SCANCODE_KP_1 + i] == 1)
				{
					currentNode = currentNode->child[i];
					nodeHasChanged = true;
					break;
				}
			}
		}

		if (nodeHasChanged)
		{
			cout << "new node: " << currentNode->size << "^3 ";
			if (currentNode->isLeaf)
			{
				cout << "leaf ";
				if (currentNode->isEmpty)
					cout << "(empty) ";
				int neighborCount = 0;
				for (uint i = 0; i < 6; ++i)
					if (currentNode->neighbors[i] != NULL)
						neighborCount++;
				cout << "(" << neighborCount << " neighbors) ";
			}
			else
				cout << "branch ";
			cout << " at " << currentNode->depth << " depth, pos: <" << currentNode->x << ", " << currentNode->y << ", " << currentNode->z << endl;
		}
	}
}

void render(double dt)
{
	glClearColor(0.6f, 0.851f, 0.918f, 1.0f);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	Shader* shader;
	shader = engine.bindShader("mainShader");

	// get a pointer to the current shader
	// = engine.getShader();
	Camera* camera = engine.getCamera();
	// get a pointer to the world matrix stack
	MatrixStack* worldMatrix = engine.getWorldMatrix();
	Matrix4 t;

	// no texturing
	shader->setUniform1i("useTexturing", 0);
	// gather all the matrices and upload
	engine.updateShaderMatrices();

	// draw that shit!
	grid.render();

	//static double angle = 0.0f;
	//static double sign = 1.0f;
	
	worldMatrix->push();
		worldMatrix->translate(5.0f, 0.0f, -5.0f);
		/*
		t.setRight(Vector3(1.0f, 0.0f, 0.0f));
		t.setUp(Vector3(0.0f, cos(angle), sin(angle)));
		t.setForward(-Vector3(0.0f, sin(angle), -cos(angle)));
		t.transpose();
		*/
		//worldMatrix->mulMatrix(t);
		engine.updateShaderMatrices();
		cube.render();
	worldMatrix->pop();

	worldMatrix->push();
		worldMatrix->translate(cubePos);
		shader->setUniform1i("useTexturing", 1);
		shader->setUniform1i("useHeightFactor", 0);
		glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);
		engine.updateShaderMatrices();
		testTexture.bind(0);

		cube2.render();
		shader->setUniform1i("useTexturing", 0);
	worldMatrix->pop();	

	

	engine.updateShaderMatrices();
	// plain ol' rendering of each segment
	if (renderMode == 1)
		engine._world.render(engine._currentShader, false);
	else if (renderMode == 2)
	{
		//engine.updateShaderMatrices();
		//t.makeTranslate(-camera->getPosition());
		engine._octree.render3(shader, worldMatrix->top());
	}
	else if (renderMode == 3)
	{
		engine._octree.render4(shader, &camera->getFrustum());
		//engine._world.render(engine._currentShader, false);
	}
	else if (renderMode == 4)
	{
		Matrix4 pvm, t;

		t.makeTranslate(-camera->getPosition());
		pvm = camera->getProjectionMatrix() * camera->getViewMatrix() * t;

		engine._octree.renderWireframes(shader, pvm, &camera->getFrustum());
	}
	else if (renderMode == 5)
	{
		Matrix4 pvm, t;

		engine._octree.render4(shader, &camera->getFrustum());

		t.makeTranslate(-camera->getPosition());
		pvm = camera->getProjectionMatrix() * camera->getViewMatrix() * t;

		engine._octree.renderNodeTest(shader, pvm, &camera->getFrustum(), cubePos, maxDepth);
	}
	else if (renderMode == 6)
	{
		Matrix4 pvm, t;

		//glBindTexture(GL_TEXTURE_2D, fontTexture);
		//testTexture.bind(0);
		engine._octree.render4(shader, &camera->getFrustum());

		t.makeTranslate(-camera->getPosition());
		pvm = camera->getProjectionMatrix() * camera->getViewMatrix() * t;

		engine._octree.renderNodeTest2(shader, pvm, &camera->getFrustum(), currentNode, highlightNode);
	}

	engine.bindFont("font1");
	engine.setFontColor(0.25f, 0.0f, 0.25f);
	engine.renderTextf(0.0f, 50.0f, "the quick brown fox jumped over the lazy dog!?~-=/\\+ 1234567890");

	engine.bindFont("font2");
	engine.setFontColor(0.5f, 0.0f, 0.0f);
	engine.renderTextf(0.0f, 150.0f, "the quick brown fox jumped over the lazy dog!?~-=/\\+ 1234567890");
	
	//engine._physics.renderEntities(shader, camera);
}

void init()
{
	cout << "starting client world init; glGetError: " << glGetError() << endl;

	engine.loadVoxelDefs("./res/defs/voxels.txt");
	engine.createWorld("./res/segments/world5.txt");
	engine.buildOctree(500);
	
	// jesus, make this neater....
	engine._physics.init(&engine._octree);
	engine._physics._entities.alloc(1);

	Entity* e = new Entity();
	e->setMass(100.0f);
	e->setPosition(128.0f, 128.0f, 128.0f);

	//box.set(Vector3(1.0f, 1.0f, -4.0f), 1.0f, 1.0f, 1.0f);
	//AABox* localBox = new AABox();
	//localBox->getType();
	//localBox->set(
	engine._physics._entities[0] = e;

	maxDepth = engine._octree._treeDepth;

	// weee?
	currentNode = engine._octree._root;
	lastNode = NULL;
	highlightNode = NULL;

	cout << "new node: " << currentNode->size << "^3 ";
	if (currentNode->isLeaf)
	{
		cout << "leaf ";
		if (currentNode->isEmpty)
			cout << "(empty) ";
		int neighborCount = 0;
		for (uint i = 0; i < 6; ++i)
			if (currentNode->neighbors[i] != NULL)
				neighborCount++;
		cout << "(" << neighborCount << " neighbors) ";
	}
	else
		cout << "branch ";
	cout << " at " << currentNode->depth << " depth, pos: <" << currentNode->x << ", " << currentNode->y << ", " << currentNode->z << ">" << endl;

	//cout << "done with client world init; glGetError: " << glGetError() << endl;
	
	renderMode = 6;

	initShaders();
	initObjects();
	initLights();
}

void initShaders()
{
	engine.createShader("mainShader", "./res/shaders/BasicLighting.vp", "./res/shaders/BasicLighting.fp");
}

void initObjects()
{
	// init cam
	Camera* cam = engine.createCamera("cam1");
	cam->setPosition(0.0f, 0.0f, 5.0f);
	cam->lookAt(Vector3(0.0f, 0.0f, -1.0f));
	cam->setProjection(1440, 900, 60.0f, 1.0f, 2000.0f);
	engine.bindCamera("cam1");

	// init fonts
	engine.createFont("font1", "./res/fonts/consola.ttf", 24);
	engine.createFont("font2", "./res/fonts/arial.ttf", 16);

	// init meshes
	cube.load("cube1", "./res/objects/cube_n.obj");
	cube.setVertexAttribIndex("pos", 0);
	cube.setVertexAttribIndex("color", 1);
	cube.build();

	cube2.load("cube2", "./res/objects/cube2.obj");
	cube2.setVertexAttribIndex("pos", 0);
	//cube2.setVertexAttribIndex("color", 1);
	cube2.setVertexAttribIndex("uv0", 3);
	cube2.build();

	testTexture.create("./res/textures/rock.bmp", 0, 0, TCF_TEXTURE | TCF_MIPMAP | TCF_FORCE_32);
	

	grid.load("grid", "./res/objects/grid.obj");
	grid.setVertexAttribIndex("pos", 0);
	grid.setVertexAttribIndex("color", 1);
	grid.build();
}

void initLights()
{
	//
}

void cleanup()
{
	if (testFont != NULL)
		delete testFont;

	engine.destroyWorld();
	cube.destroy();
	grid.destroy();
}

int main(int argCount, char *argValues[])
{
	// weee?
	cout << "begin!" << endl << endl;

	engine.setCallbacks(init, handleInput, render, cleanup);
	if (engine.init(1440, 900) == 0)
		engine.run();
	engine.cleanup();

	cout << "nice work everyone." << endl;

	return 0;
}
