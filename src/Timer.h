/*
	Hi-res timer
*/

#pragma once
#include "ChawkFortress.h"

//#include "Vector.h"
//#include "Matrix.h"
//#include "Quaternion.h"
//#include "Camera.h"
//#include "Engine.h"
//#include "Log.h"
//#include "MatrixStack.h"
//#include "Perlin.h"
//#include "SceneObject.h"
//#include "Shader.h"
//#include "Sketch.h"
//#include "Texture.h"
//#include "World.h"

class Timer
{
	public:
		bool init();
		void reset();
		float getElapsedSeconds();
		double getElapsedSeconds2();
		float getElapsedMilliseconds();
		float getFPS(unsigned int elapsedFrames);
		double getFPS2(unsigned int elapsedFrames);

	private:
		LARGE_INTEGER lastTime, lastFPSTime;
		LARGE_INTEGER ticksPerSecond;
};
