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
//#include "SceneObject.h"
//#include "Sketch.h"
//#include "Texture.h"
//#include "Timer.h"
//#include "World.h"

class Shader
{
	public:
		Shader():
		  _program(0)
		{
			  //
		}
		Shader(const char* vs, const char* fs);
		~Shader()
		{
			destroy();
		}

		void create(const char* vs, const char* fs);

		int getUniform(const char* var);
		unsigned int getProgram();

		void setUniform1f(const char* var, float x);
		void setUniform1f(unsigned int var, float x);
		void setUniform3f(const char* var, float x, float y, float z);
		void setUniform3f(unsigned int var, float x, float y, float z);

		void setUniform1i(const char* var, int x);
		void setUniform1i(unsigned int var, int x);
		
		void setMatrix4f(const char* var, float* m);
		void setMatrix4f(unsigned int var, float* m);
		void setMatrix4f(const char* var, const Matrix4& m);
		void setMatrix4f(unsigned int var, const Matrix4& m);

		void use();

		void destroy();

	private:
		unsigned int _loadShader(const char* fileName, GLenum shaderType);
		unsigned int _createProgram(unsigned int vs, unsigned int fs);

		list<unsigned int> _vertexShaders;
		list<unsigned int> _fragmentShaders;
		unsigned int _program;
		unordered_map<string, int> _uniforms;
};

