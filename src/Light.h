#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

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
#include "Mesh.h"
#include "Shader.h"

using namespace std;

class Light
{
	public:
		enum Type { NONE = 0, DIRECTIONAL = 1, POINT, SPOT };

		Light():
			_type(NONE), _program(NULL),
			_model(NULL),
			_shadowMapFBO(0), _shadowMapTexture(0),
			_bound(false),
			_pos(0.0f, 0.0f, 0.0f),
			_color(1.0f, 1.0f, 1.0f),
			_direction(0.0f, 0.0f, 0.0f),
			_ambient(0.0f), _diffuse(1.0f),
			_cutoff(0.0f),
			_a(0.0f), _b(0.0f), _c(0.0f)
		{
			//
		}

		~Light()
		{
			destroy();
		}

		void setType(Type type);

		void setPos(float x, float y, float z);
		void setPos(Vector3& pos);
		void setColor(float r, float g, float b);
		void setColor(Vector3& color);
		void setDirection(float x, float y, float z);
		void setDirection(Vector3& direction);
		void lookAt(float x, float y, float z);
		void lookAt(Vector3& target);
		void setAmbient(float ambient);
		void setDiffuse(float diffuse);
		void setCutoff(float cutoff);
		void setAttenuation(float a, float b, float c);

		void setProjection(int width, int height, float fovY, float zNear, float zFar);
		void setWorldMatrix(Matrix4& m);
		void setViewMatrix(Matrix4& m);

		void loadModel(const char* fileName);
		void render();
		void createShadowMap(int width, int height);
		void bindShadowMapForWriting();
		void bindShadowMapForReading(int index);

		void updateShader();
		void bind(Shader* program, int index);
		void unbind();
		void destroy();

		Type getType();
		Vector3 getPos();
		Vector3 getColor();
		Vector3 getDirection();
		float getAmbient();
		float getDiffuse();
		float getCutoff();

		Matrix4 getWorldMatrix() { return _worldMatrix; }
		Matrix4 getViewMatrix() { return _viewMatrix; }
		Matrix4 getProjectionMatrix() { return _projectionMatrix; }		

	//private:
		void _computeViewMatrix();
		enum Parameter { COLOR = 0, AMBIENT, DIFFUSE, DIRECTION, POS, A, B, C, CUTOFF };

		Type _type;
		Shader* _program;
		Mesh* _model;
		Matrix4 _projectionMatrix, _viewMatrix, _worldMatrix;
		int _wvpMatrixLoc;
		unsigned int _shadowMapFBO;
		unsigned int _shadowMapTexture;
		bool _bound;
		Vector3 _pos, _color, _direction;
		float _ambient, _diffuse, _cutoff;
		float _a, _b, _c;
		int _uniforms[9];
};

