/*
	generic "scene object" class that runs on GL3 VAO's, VBO's and IBO's

*/

#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Camera.h"
//#include "Engine.h"
//#include "Log.h"
#include "MatrixStack.h"
//#include "Perlin.h"
//#include "Shader.h"
//#include "Sketch.h"
#include "Texture.h"
//#include "Timer.h"
//#include "World.h"

class Mesh
{
	public:
		// hold info about each vertex attrib data stream, mapped to via a std::string
		struct Buffer
		{
			float* data;
			unsigned int index, len, offset;
			int componentCount;
			int elementCount;
		};

		Mesh():
			_position(0.0f, 0.0f, 0.0f),
			_vbo(0), _ibo(0), _vao(0),
			_iboData(NULL), _iboDataLen(0),
			_elementCount(0), _iboDataType(0),
			_usingIBO(false), _renderMode(0)
		{
			//
		}

		~Mesh()
		{
			destroy();
		}

		void load(const char* name, const char* fileName);
		void addBuffer(const char* name, unsigned int index, int componentCount, int stride, float* data, unsigned int dataCount);
		Buffer getBufferData(const char* name);
		void setIndexBuffer(GLenum renderMode, GLenum dataType, void* data, unsigned int dataCount);
		void computeNormals(const char* streamName);
		void build();
		void render();
		void saveToFile(const char* fileName);
		void destroy();

		void setName(const char* name);
		void setRenderMode(GLenum renderMode);
		void setVertexAttribIndex(const char* buffer, unsigned int index);

	private:
		void _computeNormalFromTriangle(float* in0, float* in1, float* in2, float* out);

		// basic object parameters
		string _name;
		Vector3 _position;
		//Quaternion _orientation;

		// rendery stuff
		unsigned int _vbo, _ibo, _vao;
		unordered_map<string, Buffer> _buffers;

		// this either holds the count of n-tuples defining vertex attributes, per buffer,
		// or the number of indices to render with when using an IBO
		unsigned int _elementCount;

		// index buffer stuff (optional)
		bool _usingIBO;
		void* _iboData;
		GLenum _iboDataType;
		unsigned int _iboDataLen;
		
		// how to draw the data!
		GLenum _renderMode;
};
