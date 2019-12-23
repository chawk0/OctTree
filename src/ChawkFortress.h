/*
	Main include file for every other class.  Contains some custom types, class decl's and common structs.  Weee
*/

#pragma once

#include <Windows.h>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdarg>
#include <ctime>
#include <iostream>
#include <fstream>
#include <map>
#include <new>
#include <string>
#include <unordered_map>
#include <vector>

#include <gl/glew.h>
#include <gl/gl.h>
//#include <gl/glu.h>
#include <sdl/SDL.h>
#include <sdl/SDL_image.h>

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace std;

#ifndef int64
typedef long long int64;
#endif
#ifndef uint64
typedef unsigned long long uint64;
#endif
#ifndef uint
typedef unsigned int uint;
#endif

#define CHECKGLERROR(func)	{int __e = glGetError(); if (__e) { cout << endl << "ERROR: " #func ": " << __e << endl; }}

class Vector2;
class Vector3;
class Matrix4;
class Quaternion;
class Plane;

class Camera;
class Engine;
class Log;
class MatrixStack;
class Perlin;
class Mesh;
class Shader;
class Sketch;
class Texture;
class Timer;
class World;

#include "Array.h"
#include "DualLinkedList.h"

// holds properties of each type of voxel (soil, water, etc).  these are held in
// an Array<Voxel> inside Engine, and the id of a voxel is equal to its index in the array.
struct Voxel
{
	string name;
	string texturePath;
	Texture* texture;
};

// holds properties of a vertex attribute.
struct VertexAttrib
{
	string name;
	uint index;			// the index of vertex attrib data stream that is fed to the shader
	int componentCount;	// a position, for example, has componentCount = 3 (x, y and z)
	GLenum dataType;	// a GL constant specifying type of data for this attribute
	int stride;			// stride between successive vertex attribs in the associated buffer
	void* offset;		// offset into the associated buffer where the vertex attrib data starts
};
		
// holds properties of a VBO, including a GL handle to the VBO and optional client-side buffer pointer
struct VertexBuffer
{
	uint vbo;			// the GL handle returned by glGenBuffers
	uint target;		// typically GL_ARRAY_BUFFER
	uint size;			// size in bytes
	float* data;		// buffer in system memory.  this is copied by the driver into vram (usually)
	uint usage;			// typically GL_STATIC_DRAW
};

// holds properties of an IBO, including a GL handle to the IBO and optional client-side buffer pointer
struct IndexBuffer
{
	uint ibo;			// the GL handle returned by glGenBuffers
	uint target;		// typically GL_ELEMENT_ARRAY_BUFFER
	uint size;			// size in bytes
	void* data;			// buffer in system memory.  this is copied by the driver into vram (usually)
	GLenum dataType;	// either GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, or GL_UNSIGNED_INT
	uint usage;			// typically GL_STATIC_DRAW
};

// meh?  I guess this works.  encapsulates all the GL render objects and handles
struct RenderData
{
	uint vao;
	Array<VertexBuffer> vertexBuffers;
	Array<IndexBuffer> indexBuffers;
	Array<VertexAttrib> vertexAttribs;
	uint elementCount;
};

// represents a portion of the world that is rendered as one unit (terrain layer, water layer, etc)
struct Segment
{
	enum DataSourceType { NONE = 0, FILE = 1, PERLIN = 2 };

	string name;
	uint voxelID;		// the id of the voxel this segment is composed of
	Array<uint> visibleThroughIDs;	// array of voxel IDs that this segment is visible through
	bool useBlend;		// whether to enable GL_BLEND when rendering this segment
	DataSourceType dataSourceType;	// specifies the source of the data for the segment (file, Perlin noise, etc)
	string dataSourceFile;			// if data is loaded from file, this is the path
	double perlinFrequency, perlinPersistence, amplitude;	// if data is Perlin noise, these are the parameters
	uint visibleFaceCount;
};

