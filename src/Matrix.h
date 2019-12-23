/*
	column-major float-based 4x4 Matrix class
*/

#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "Quaternion.h"
//#include "Camera.h"
//#include "Engine.h"
//#include "Log.h"
//#include "MatrixStack.h"
//#include "Perlin.h"
//#include "SceneObject.h"
//#include "Shader.h"
//#include "Sketch.h"
//#include "Texture.h"
//#include "Timer.h"
//#include "World.h"

class Matrix4
{
	public:
		float m[16];

		Matrix4() { }
		Matrix4(float *n) { if (n) memcpy(m, n, sizeof(float) * 16); }
		Matrix4(const Matrix4& n) { memcpy(m, n.m, sizeof(float) * 16); }

		Matrix4& operator=(const Matrix4& n)
        {
            memcpy(m, n.m, sizeof(float) * 16);
            return *this;
        }
        
        bool operator==(const Matrix4& n) const
        {
            return (memcmp(m, n.m, sizeof(float) * 16) == 0);
        }
        
        bool operator!=(const Matrix4& n) const
        {
            return !(*this == n);
        }
        
        Matrix4 operator+(const Matrix4& n) const
        {
            Matrix4 result;
            
            for (int i = 0; i < 16; i++)
                result.m[i] = m[i] + n.m[i];

            return result;
        }
        
        Matrix4& operator+=(const Matrix4& n)
        {
            for (int i = 0; i < 16; i++)
                m[i] += n.m[i];

            return *this;
        }
        
        Matrix4 operator-(const Matrix4& n) const
        {
            Matrix4 result;
            
            for (int i = 0; i < 16; i++)
                result.m[i] = m[i] - n.m[i];

            return result;
        }

        Matrix4& operator-=(const Matrix4& n)
        {
            for (int i = 0; i < 16; i++)
                m[i] -= n.m[i];

            return *this;
        }
        
        Matrix4 operator-() const
        {
            Matrix4 result;
            
            for (int i = 0; i < 16; i++)
                result.m[i] = -m[i];

            return result;
        }
        
        Matrix4 operator*(float s) const
        {
            Matrix4 result;
            
            for (int i = 0; i < 16; i++)
                result.m[i] = m[i] * s;

            return result;
        }
        
        Vector3 operator*(const Vector3& v) const
        {
            Vector3 result;
           
			// assumes the input vec3 is <x,y,z,1>
            result.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12];
            result.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13];
            result.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];
            
            return result;
        }
        
		// float*-based multiply. bleh?
        void vectorMult(float* v, float* result)
        {
            result[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12];
            result[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13];
            result[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
		}
        
        Matrix4 operator*(const Matrix4& n) const
        {
            Matrix4 result;
            
            for (int row = 0; row < 4; row++)
                for (int col = 0; col < 4; col++)
                {
                    result.m[col * 4 + row] = m[0 + row] * n.m[0 + col * 4] +
						                      m[4 + row] * n.m[1 + col * 4] +
											  m[8 + row] * n.m[2 + col * 4] +
											  m[12 + row] * n.m[3 + col * 4];
                }
                
            return result;
        }
        
        Matrix4& operator*=(float s)
        {
            for (int i = 0; i < 16; i++)
                m[i] *= s;
                
            return *this;
        }
        
        Matrix4& operator*=(const Matrix4& n)
        {
            float temp[16];

            for (int row = 0; row < 4; row++)
                for (int col = 0; col < 4; col++)
                {
                    temp[col * 4 + row] = m[0 + row] * n.m[0 + col * 4] +
                                          m[4 + row] * n.m[1 + col * 4] +
                                          m[8 + row] * n.m[2 + col * 4] +
                                          m[12 + row] * n.m[3 + col * 4];
                }

            memcpy(m, temp, sizeof(float) * 16);
            
            return *this;
        }
        
        Matrix4 operator/(float s) const
        {
            Matrix4 result;
			float i;

            if (s != 0.0f)
            {
				i = 1.0f / s;
                for (int i = 0; i < 16; i++)
                    result.m[i] = m[i] * i;
            }
            else
				printf("error: Matrix4::operator /: divide by zero!\n");
            
			return result;
        }
        
        Matrix4& operator/=(float s)
        {
			float i;
            if (s != 0.0f)
            {
				i = 1.0f / s;
                for (int i = 0; i < 16; i++)
                    m[i] *= i;
            }
            else
				printf("error: Matrix4::operator /=: divide by zero!\n");
            
            return *this;
        }

		Matrix4& setRight(const Vector3& v)
		{
			m[0] = v.x;
			m[4] = v.y;
			m[8] = v.z;
			return *this;
		}

		Matrix4& setUp(const Vector3& v)
		{
			m[1] = v.x;
			m[5] = v.y;
			m[9] = v.z;
			return *this;
		}

		Matrix4& setForward(const Vector3& v)
		{
			m[2] = v.x;
			m[6] = v.y;
			m[10] = v.z;
			return *this;
		}

        Matrix4& transpose()
        {
        	/*| 0  4  8 12 |
         	  | 1  5  9 13 |
          	  | 2  6 10 14 |
          	  | 3  7 11 15 |*/
            _swap(m[1],  m[4] );
            _swap(m[2],  m[8] );
            _swap(m[3],  m[12]);
            _swap(m[6],  m[9] );
            _swap(m[7],  m[13]);
            _swap(m[11], m[14]);

            return *this;
        }

        Matrix4& makeRotateX(float angle)
		{
			float ca = cosf(PI_OVER_180 * angle);
			float sa = sinf(PI_OVER_180 * angle);

			makeIdentity();
			m[5] = ca; m[9] = -sa;
			m[6] = sa; m[10] = ca;

			return *this;
		}

        Matrix4& makeRotateY(float angle)
		{
			float ca = cosf(PI_OVER_180 * angle);
			float sa = sinf(PI_OVER_180 * angle);

			makeIdentity();
			m[0] = ca; m[8] = sa;
			m[2] = -sa; m[10] = ca;

			return *this;
		}

        Matrix4& makeRotateZ(float angle)
		{
			float ca = cosf(PI_OVER_180 * angle);
			float sa = sinf(PI_OVER_180 * angle);

			makeIdentity();
			m[0] = ca; m[4] = -sa;
			m[1] = sa; m[5] = ca;

			return *this;
		}

        Matrix4& makeTranslate(float tx, float ty, float tz)
		{
			makeIdentity();
			m[12] = tx; m[13] = ty; m[14] = tz;

			return *this;
		}

		Matrix4& makeTranslate(float* v)
		{
			makeIdentity();
			m[12] = v[0]; m[13] = v[1]; m[14] = v[2];

			return *this;
		}

		Matrix4& makeTranslate(Vector3& v)
		{
			makeIdentity();
			m[12] = v.x; m[13] = v.y; m[14] = v.z;

			return *this;
		}

        Matrix4& makeScale(float sx, float sy, float sz)
		{
			makeIdentity();
			m[0] = sx; m[5] = sy; m[10] = sz;

			return *this;
		}

		Matrix4& makeProjection(float aspectRatio, float fovY, float nearZ, float farZ)
		{
			float tanHalfFov =	tanf(fovY * PI_OVER_180 / 2);
			float zRange = farZ - nearZ;
			//float zRange = nearZ - farZ;
			
			
			m[0] = 1.0f / (aspectRatio * tanHalfFov); m[4] = 0.0f;				m[8] = 0.0f;					  m[12] = 0.0f; 
			m[1] = 0.0f;							  m[5] = 1.0f / tanHalfFov; m[9] = 0.0f;					  m[13] = 0.0f; 
			m[2] = 0.0f;							  m[6] = 0.0f;				m[10] = -(farZ + nearZ) / zRange; m[14] = -2.0f * farZ * nearZ / zRange;
			m[3] = 0.0f;							  m[7] = 0.0f;				m[11] = -1.0f;					  m[15] = 0.0f;
			
			/*
			m[0] = 1.0f / (aspectRatio * tanHalfFov); m[4] = 0.0f;				m[8] = 0.0f;					  m[12] = 0.0f; 
			m[1] = 0.0f;							  m[5] = 1.0f / tanHalfFov; m[9] = 0.0f;					  m[13] = 0.0f; 
			m[2] = 0.0f;							  m[6] = 0.0f;				m[10] = (farZ + nearZ) / zRange; m[14] = 2.0f * farZ * nearZ / zRange;
			m[3] = 0.0f;							  m[7] = 0.0f;				m[11] = -1.0f;					  m[15] = 0.0f;
			*/
			return *this;
		}

		Matrix4& makeView(Vector3& forward)
		{
			Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
			Vector3 newRight, newUp;
			
			if (forward.isEqualEpsilon(up))
				up = Vector3(1.0f, 0.0f, 0.0f);

			newRight = forward.cross(up);
			newUp = newRight.cross(forward);
			newRight.normalize();
			newUp.normalize();

			setRight(newRight);
			setUp(newUp);
			setForward(forward);

			return *this;
		}

		Matrix4& makeOrtho(float left, float right, float top, float bottom, float zNear, float zFar)
		{
			makeZero();
			m[0] = 2.0f / (right - left);
			m[5] = 2.0f / (top - bottom);
			m[10] = -2.0f / (zFar - zNear);
			m[12] = -(right + left) / (right - left);
			m[13] = -(top + bottom) / (top - bottom);
			m[14] = -(zFar + zNear) / (zFar - zNear);
			m[15] = 1.0f;
			return *this;
		}

        Matrix4& makeIdentity()
		{
			makeZero();
			m[0] = 1.0f; m[5] = 1.0f; m[10] = 1.0f; m[15] = 1.0f;

			return *this;
		}

        Matrix4& makeZero()
		{
			memset(m, 0, sizeof(float) * 16);

			return *this;
		}

		Quaternion toQuaternion();

	private:
		void _swap(float& a, float& b)
		{
			float t;
			t = a; a = b; b = t;
		}
};

ostream& operator<<(ostream& os, const Matrix4& m);


