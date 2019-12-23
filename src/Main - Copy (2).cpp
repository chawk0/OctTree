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

using namespace std;


//------------------------------------------------------------------------------

SDL_Window* window = NULL;
SDL_GLContext glContext = 0;
Uint8* keyStates = NULL;

Camera camera;
Matrix4 projectionMatrix;
ShaderProgram program;
int projectionMatrixLoc, viewMatrixLoc, modelMatrixLoc;

//Sketch sketch;
SceneObject cube2;

unsigned int VAOs[3];
unsigned int VBOs[3];
unsigned int IBOs[2];

unsigned int shaderProgram;

Texture baseMap, normalMap;
//unsigned int baseMap;
//unsigned int normalMap;
//unsigned int samplers[2];
int baseMapLoc;
int normalMapLoc;
int staticColorLoc;	// the static color for rendering arrays that lack color data
int lightPosLoc;

int currentFace = 0;


#define CUBE	0
#define PYRAMID	1
#define GRID	2
#define GRID_LENGTH	40

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
	
// plain vertex list, not indexed
float pyramidData[] = {-1.0f, -0.4082483f,  0.5773503f, 0.0f, 0.0f,	// front
						1.0f, -0.4082483f,  0.5773503f, 1.0f, 0.0f,
						0.0f,  1.2247449f,        0.0f, 0.5f, 1.0f,

						1.0f, -0.4082483f,  0.5773503f, 0.0f, 0.0f,	// right
						0.0f, -0.4082483f, -1.1547005f, 1.0f, 0.0f,
						0.0f,  1.2247449f,        0.0f, 0.5f, 1.0f,

						0.0f, -0.4082483f, -1.1547005f, 0.0f, 0.0f,	// left
						-1.0f, -0.4082483f,  0.5773503f, 1.0f, 0.0f,
						0.0f,  1.2247449f,        0.0f, 0.5f, 1.0f,

						-1.0f, -0.4082483f,  0.5773503f, 0.0f, 0.0f,	// bottom
						0.0f, -0.4082483f, -1.1547005f, 1.0f, 0.0f,
						1.0f, -0.4082483f,  0.5773503f, 0.5f, 1.0f,
	
						// 108 floats for the TBN matrices
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// grid_length of N means a grid from -N to N, which is (grid_length + 1) lines
// line count times 2 for X and Z directions, add 3 for the origin axes
// then times 2 vertices per line, times 6 floats per vertex (x, y, z, r, g, b)
float gridData[(2 * (GRID_LENGTH + 1) + 3) * 2 * 6];

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
void render();
void handleInput();

//------------------------------------------------------------------------------

// class funcs?

//------------------------------------------------------------------------------

void handleInput()
{
	keyStates = (Uint8*)SDL_GetKeyboardState(NULL);

	if (keyStates[SDL_SCANCODE_D] == 1)
		camera.translateX(0.1f);
	else if (keyStates[SDL_SCANCODE_A] == 1)
		camera.translateX(-0.1f);

	if (keyStates[SDL_SCANCODE_W] == 1)
		camera.translateY(0.1f);
	else if (keyStates[SDL_SCANCODE_S] == 1)
		camera.translateY(-0.1f);

	if (keyStates[SDL_SCANCODE_X] == 1)
		camera.translateZ(0.1f);
	else if (keyStates[SDL_SCANCODE_Z] == 1)
		camera.translateZ(-0.1f);

	if (keyStates[SDL_SCANCODE_E] == 1)
		camera.rotateZ(2.0f);
	else if (keyStates[SDL_SCANCODE_Q] == 1)
		camera.rotateZ(-2.0f);
}

void render()
{
	static MatrixStack t;
	static Matrix4 m;
	static Vector3 lightPos;
	static float angle = 0.0f;
	static int viewMatrixLoc;
	
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//glUseProgram(shaderProgram);
	//program.use();
	program.setMatrix4f("projectionMatrix", projectionMatrix);
	
	// apply cam transform
	t.loadMatrix(camera.getViewMatrix());
	t.translate(-camera.getPosition());
	t.apply(program.getUniform("viewMatrix"));

	// ready the model-to-world matrix
	t.makeIdentity();

	/*
	// CUBE
	t.push();
		t.translate(-5.0f, 2.0f, -5.0f);
		t.rotateY(angle);
		t.apply(program.getUniform("modelMatrix"));
		
		// the one VAO saves the buffer and vertex attrib pointer bindings in one quick call, neat.
		glBindVertexArray(VAOs[CUBE]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
	t.pop();

	// CUBE2, NEW AND IMPROVED
	t.push();
		t.translate(-10.0f, 2.0f, -5.0f);
		t.rotateY(angle);
		t.apply(program.getUniform("modelMatrix"));
		
		// the one VAO saves the buffer and vertex attrib pointer bindings in one quick call, neat.
		cube2.render();
	t.pop();
	*/

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

		glBindVertexArray(VAOs[PYRAMID]);
		//glUniform1i(modeLoc, 0);
		//glUniform3f(staticColorLoc, 1.0f, 1.0f, 1.0f);
		program.setUniform3f("staticColor", 1.0f, 1.0f, 1.0f);
		glDrawArrays(GL_TRIANGLES, 0, 12);

		//glUniform3f(staticColorLoc, 0.0f, 0.0f, 0.0f);
		program.setUniform3f("staticColor", 0.0f, 0.0f, 0.0f);
	t.pop();
	

	// PYRAMID
	// setup tex units
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, baseMap);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, normalMap);
	baseMap.bind(0);
	normalMap.bind(1);
	
	// set baseMap sampler to tex unit 0, normalMap sampler to tex unit 1
	//glUniform1i(baseMapLoc, 0);
	//glUniform1i(normalMapLoc, 1);
	//glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	program.setUniform1i("baseMap", 0);
	program.setUniform1i("normalMap", 1);
	program.setUniform3f("lightPos", lightPos.x, lightPos.y, lightPos.z);

	t.push();
		t.translate(5.0f, 2.0f, -5.0f);
		t.apply(program.getUniform("modelMatrix"));

		glBindVertexArray(VAOs[PYRAMID]);
		glDrawArrays(GL_TRIANGLES, 0, 12);
	t.pop();

	// reset tex units, don't forget!
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, 0);
	baseMap.unbind(0);
	normalMap.unbind(1);

	/*
	// TBNs
	t.push();
		//t.translate(5.0f, 2.0f, -5.0f);
		t.apply(modelMatrixLoc);
		// create a separate sketch to draw each face's TBN
		sketch.begin(GL_LINES, (char*)(string("tbn") + string(1, (char)(currentFace + 0x30))).c_str());
			float* v;
			float* tbn;
			v = &pyramidData[0 + currentFace * 3 * 5];
			tbn = &pyramidData[60 + currentFace * 3 * 1 * 9];

			// 3 TBNs per face
			for (int i = 0; i < 3; i++)
			{
				// T vector in red
				sketch.color3f(1.0f, 0.0f, 0.0f);
				sketch.vertex3fv(v);
				sketch.vertex3(Vector3(v) + 0.1f * Vector3(tbn));
				tbn += 3;

				// b vector in green
				sketch.color3f(0.0f, 1.0f, 0.0f);
				sketch.vertex3fv(v);
				sketch.vertex3(Vector3(v) + 0.1f * Vector3(tbn));
				tbn += 3;

				// N vector in blue
				sketch.color3f(0.0f, 0.0f, 1.0f);
				sketch.vertex3fv(v);
				sketch.vertex3(Vector3(v) + 0.1f * Vector3(tbn));
				tbn += 3;
				v += 5;
			}
		sketch.end();
	t.pop();
	*/

	// GRID
	t.apply(program.getUniform("modelMatrix"));
	glBindVertexArray(VAOs[GRID]);
	// draw the XZ grid and the 3 origin axes
	glDrawArrays(GL_LINES, 0, 2 * (2 * (GRID_LENGTH + 1) + 3));
	
	/*
	// yay sketch
	t.apply(modelMatrixLoc);
	sketch.begin(GL_TRIANGLES, "wee");
		// don't technically need these, but that's the beauty
		sketch.color3f(1.0f, 1.0f, 0.0f);
		sketch.vertex3f(-1.0f, -1.0f, 0.0f);
		sketch.vertex3f( 1.0f, -1.0f, 0.0f);
		sketch.vertex3f( 0.0f,  1.0f, 0.0f);
	sketch.end();

	// one more.  some GRASS
	t.push();
		t.translate(-5.0f, 0.0f, 0.0f);
		t.apply(modelMatrixLoc);	

		sketch.begin(GL_TRIANGLES, "wee too");
			for (int i = 0; i < 16; i++)
			{
				float r = (float)(i % 4) / 3.0f;
				float g = 1.0f - (float)(i / 4) / 3.0f;
				float b = 0.0f;//0.5 * sinf(sqrt((float)((i % 4) * (i % 4) + (i / 4) * (i / 4)) / 4.0f / sqrt(2.0f) * 2.0f * PI));
				float dx = (float)(i % 4) - 1.5f;
				float dz = -(float)(i / 4) + 1.5f;

				sketch.color3f(r, g, b);
				sketch.vertex3f(-1.0f + dx, -1.0f, 0.0f + dz);
				sketch.vertex3f( 1.0f + dx, -1.0f, 0.0f + dz);
				sketch.vertex3f( 0.0f + dx,  1.0f, 0.0f + dz);
			}
		sketch.end();
	t.pop();
	*/
	
	// housekeeping?
	//glBindVertexArray(0);
	//glUseProgram(0);
	
	angle += 1.0f;
	if (angle >= 360.0f)
		angle -= 360.0f;

    SDL_GL_SwapWindow(window);
}

void initScene()
{
	printf("creating objects...\n");
	initObjects();
	printf("createing buffers...\n");
	initBuffers();
	
	printf("creating shaders and program...\n");
	initShaders();

	// init cam
	projectionMatrix.makeProjection(1440.0f / 900.0f, 60.0f, 0.5f, 1000.0f);
	camera.setPosition(0.0f, 3.0f, 8.0f);

	// set the sketch to ready*/
	Sketch::init();

	baseMap.create("./res/textures/stone_base.tga");
	normalMap.create("./res/textures/stone_normal.tga");
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
	// generate a TBN matrix for each vertex of pyramid
	float* vPtr;
	float* tbnPtr;
	float tbnOut[9];

	for (int i = 0; i < 4; i++)
	{
		// 15 floats per face, 27 floats per TBN which start after the initial 60 verts
		vPtr = &pyramidData[i * 15];
		tbnPtr = &pyramidData[i * 27 + 60];

		//copy the tbn into each of the 3 vertices of the face
		generateFaceTBN(vPtr, vPtr + 5, vPtr + 10, &tbnOut[0], &tbnOut[3], &tbnOut[6]);
		memcpy(tbnPtr, tbnOut, sizeof(float) * 9);
		tbnPtr += 9;
		memcpy(tbnPtr, tbnOut, sizeof(float) * 9);
		tbnPtr += 9;
		memcpy(tbnPtr, tbnOut, sizeof(float) * 9);
	}

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
	/*
	float cube3Data[] = {-1.0f, -1.0f,  1.0f,
						  1.0f, -1.0f,  1.0f,
						  1.0f,  1.0f,  1.0f,
						 -1.0f,  1.0f,  1.0f,
						 
						 -1.0f, -1.0f, -1.0f,
						  1.0f, -1.0f, -1.0f,
						  1.0f,  1.0f, -1.0f,
						 -1.0f,  1.0f, -1.0f};

	float cube3Colors[] = {1.0f, 0.0f, 0.0f,
						  0.0f, 1.0f, 0.0f,
						  0.0f, 0.0f, 1.0f,
						  1.0f, 1.0f, 1.0f,
						  1.0f, 0.0f, 0.0f,
						  1.0f, 1.0f, 1.0f,
						  1.0f, 0.0f, 0.0f,
						  0.0f, 1.0f, 0.0f};
	// triangle index data
	unsigned short cube3Indices[] = {0, 1, 2, 2, 3, 0,	// front
									3, 2, 6, 6, 7, 3,	// top
									1, 5, 6, 6, 2, 1,	// right
									4, 0, 3, 3, 7, 4,	// left
									5, 4, 7, 7, 6, 5,	// back
									4, 5, 1, 1, 0, 4};	// bottom
	SceneObject cube3;
	
	cube3.setName("cube3");
	cube3.addBuffer("verts", 0, 3, 0, cube3Data, 8 * 3);

	float* ptr = cube3Colors;
	for (int i = 0; i < 8; ++i)
	{
		*(ptr + 0) = (float)(rand() % 256) / 255.0f;
		*(ptr + 1) = (float)(rand() % 256) / 255.0f;
		*(ptr + 2) = (float)(rand() % 256) / 255.0f;
		ptr += 3;
	}
	cube3.addBuffer("colors", 1, 3, 0, cube3Colors, 8 * 3);
	cube3.setIndexBuffer(GL_TRIANGLES, GL_UNSIGNED_SHORT, cube3Indices, 36);
	cube3.build();
	//cube3.saveToFile("./res/objects/cube3.obj");
	
	cube2.load("cube2", "./res/objects/cube3.obj");
	cube2.setVertexAttribIndex("verts", 0);
	cube2.setVertexAttribIndex("colors", 1);
	cube2.build();
	*/
}

void initBuffers()
{
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

	// pyramid
	glBindVertexArray(VAOs[PYRAMID]);
	// 2 = uv
	VBOs[PYRAMID] = loadBuffer(GL_ARRAY_BUFFER, pyramidData, sizeof(pyramidData));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0); 
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (GLvoid*)(sizeof(float) * 3));
	// 3 = tangent, 4 = binormal, 5 = normal
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (GLvoid*)(sizeof(float) * 60));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (GLvoid*)(sizeof(float) * 63));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (GLvoid*)(sizeof(float) * 66));
	
	// grid
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

void initShaders()
{
	program.create("./res/shaders/temp.vs", "./res/shaders/temp.fs");
	program.use();

	printf("\tprojectionMatrix loc: %d\n", program.getUniform("projectionMatrix"));
	printf("\tviewMatrix loc: %d\n", program.getUniform("viewMatrix"));
	printf("\tmodelMatrix loc: %d\n", program.getUniform("modelMatrix"));

	printf("\tstaticColor loc: %d\n", program.getUniform("staticColor"));

	printf("\tbaseMap loc: %d\n", program.getUniform("baseMap"));
	printf("\tnormalMap loc: %d\n", program.getUniform("normalMap"));
	printf("\tlightPos loc: %d\n", program.getUniform("lightPos"));

#if 0
	unsigned int vs, fs;
	vs = loadShader("./res/shaders/temp.vs", GL_VERTEX_SHADER);
	fs = loadShader("./res/shaders/temp.fs", GL_FRAGMENT_SHADER);
	shaderProgram = createProgram(vs, fs);

	glUseProgram(shaderProgram);
	
	glDetachShader(shaderProgram, vs);
	glDetachShader(shaderProgram, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);
	
	// get various shader variable handles
	projectionMatrixLoc = glGetUniformLocation(shaderProgram, "projectionMatrix");
	viewMatrixLoc = glGetUniformLocation(shaderProgram, "viewMatrix");
	modelMatrixLoc = glGetUniformLocation(shaderProgram, "modelMatrix");

	staticColorLoc = glGetUniformLocation(shaderProgram, "staticColor");

	baseMapLoc = glGetUniformLocation(shaderProgram, "baseMap");
	normalMapLoc = glGetUniformLocation(shaderProgram, "normalMap");
	lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
#endif
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

	SDL_GL_SetSwapInterval(1);
	
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
	float mouseDX, mouseDY;

	SDL_SetRelativeMouseMode(SDL_TRUE);

	printf("starting main loop...\n");
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

		handleInput();
        render();
		//printf("\tgl error %d\n", glGetError());
    }

	SDL_SetRelativeMouseMode(SDL_FALSE);

	SDL_Quit();
    return 0;
}
