/*
	basic stack of Matrix4f's to act like the old GL matrix stack
*/

#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "Matrix.h"
//#include "Quaternion.h"
//#include "Camera.h"
//#include "Engine.h"
//#include "Log.h"
//#include "Perlin.h"
//#include "SceneObject.h"
//#include "Shader.h"
//#include "Sketch.h"
//#include "Texture.h"
//#include "Timer.h"
//#include "World.h"

class MatrixStack
{
	public:
		MatrixStack() { _stack.push_back(Matrix4().makeIdentity()); }
		~MatrixStack() { }

		void translate(float x, float y, float z)
		{
			_stack.back() *= Matrix4().makeTranslate(x, y, z);
		}

		void translate(float* v)
		{
			_stack.back() *= Matrix4().makeTranslate(v);
		}

		void translate(Vector3& v)
		{
			_stack.back() *= Matrix4().makeTranslate(v);
		}

		void setWorldPos(float x, float y, float z)
		{
			_stack.back().m[12] = x;
			_stack.back().m[13] = y;
			_stack.back().m[14] = z;
		}

		void setWorldPos(float* v)
		{
			_stack.back().m[12] = v[0];
			_stack.back().m[13] = v[1];
			_stack.back().m[14] = v[2];
		}

		void setWorldPos(Vector3& v)
		{
			_stack.back().m[12] = v.x;
			_stack.back().m[13] = v.y;
			_stack.back().m[14] = v.z;
		}

		void rotate(float angle, float x, float y, float z) { }
		void rotate(float angle, Vector3& axis) { }
		void rotate(float angle, float* axis) { }

		void rotateX(float angle)
		{
			_stack.back() *= Matrix4().makeRotateX(angle);
		}

		void rotateY(float angle)
		{
			_stack.back() *= Matrix4().makeRotateY(angle);
		}

		void rotateZ(float angle)
		{
			_stack.back() *= Matrix4().makeRotateZ(angle);
		}

		void scale(float x, float y, float z)
		{
			_stack.back() *= Matrix4().makeScale(x, y, z);
		}

		void scale(float* v) { }
		void scale(Vector3& v) { }

		void makeIdentity()
		{
			_stack.back().makeIdentity();
		}

		void push()
		{
			Matrix4 m = _stack.back();
			_stack.push_back(m);
		}

		void pop()
		{
			if (_stack.size() > 1)
				_stack.pop_back();
		}

		Matrix4 top()
		{
			return _stack.back();
		}

		void loadMatrix(Matrix4& m)
		{
			_stack.back() = m;
		}

		void mulMatrix(Matrix4& m)
		{
			_stack.back() *= m;
		}

		void apply(unsigned int location)
		{
			glUniformMatrix4fv(location, 1, GL_FALSE, _stack.back().m);
		}

	private:
		vector<Matrix4> _stack;
};