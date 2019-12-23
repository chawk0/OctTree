#pragma once
#include "ChawkFortress.h"

//#include "Vector.h"
//#include "Matrix.h"
//#include "Quaternion.h"
//#include "Camera.h"
//#include "Engine.h"
//#include "MatrixStack.h"
//#include "Perlin.h"
//#include "SceneObject.h"
//#include "Shader.h"
//#include "Sketch.h"
//#include "Texture.h"
//#include "Timer.h"
//#include "World.h"

#define LOG_COMPILE

#ifndef LOG_COMPILE
	#define V(...)	0;
	#define Vf(...)	0;
	#define E(...)	0;
	#define Ef(...)	0;
#else
	#define V   Log::verbose
	#define Vf  Log::verbosef
	#define E   Log::error
	#define Ef  Log::errorf
#endif

class Log
{
	public:
		static void dumpVerboseToConsole()
		{
			FILE *h;
			freopen_s(&h, "CON", "w", stdout);
			freopen_s(&h, "CON", "w", stderr);
		}
		static void verbose(const char* v);
		static void verbosef(const char* v, ...);
		static void error(const char* e);
		static void errorf(const char* e, ...);
	        
		static char* SDLEvents[];
        
    private:
        static char _buffer[1024];
};

