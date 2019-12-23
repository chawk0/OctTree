#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
//#include "Camera.h"
//#include "Engine.h"
//#include "Log.h"
#include "MatrixStack.h"
//#include "Perlin.h"
//#include "SceneObject.h"
//#include "Shader.h"
//#include "Sketch.h"
#include "Texture.h"
//#include "Timer.h"
//#include "World.h"

class Sketch
{
	public:
		Sketch() { }
		~Sketch() { }

		//static void init();

		static void begin(GLenum mode, char* name);
		//static void begin(GLenum mode, char* name, bool renderImmediately);

		static void vertex3f(float x, float y, float z);
		static void vertex3fv(float* v);
		static void vertex3(Vector3& v);

		static void color3f(float r, float g, float b);
		static void color3fv(float* c);
		static void color3(Vector3& c);

		static void end();

	private:
		typedef struct
		{
			unsigned int _vao, _vbo;
			int _elementCount;
			GLenum _renderMode;
			string _name;
			Vector3 _currentColor;
			vector<float> _vertexData;
			vector<float> _colorData;
		} Instance;

		enum State { BUILDING = 1, READY = 2, SKIPPING = 3 };

		static int _state;
		static unordered_map<string, Instance> _instances;
		static unordered_map<string, Instance>::iterator _current;
};