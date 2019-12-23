/*
    Perlin noise generation class.
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
//#include "SceneObject.h"
//#include "Shader.h"
//#include "Sketch.h"
//#include "Texture.h"
//#include "Timer.h"
//#include "World.h"

class Perlin
{
    public:
        static void init();
		static void init(unsigned int seed);
        static double noise1D(unsigned int x, unsigned int n);
        static double noise2D(unsigned int x, unsigned int y, unsigned int n);
        static double cubicInterpNoise1D(double x, unsigned int n);
        static double cubicInterpNoise2D(double x, double y, unsigned int n);
        static double cubicInterp(double p1, double p2, double p3, double p4, double x);
        static double PerlinNoise1D(double x, double frequency, double persistance);
        static double PerlinNoise2D(double x, double y, double frequency, double persistance);

    private:
        static unsigned int randBase[10];
        static unsigned int tinyPrimes[10];
        static unsigned int lowPrimes[10];
        static unsigned int midPrimes[10];
        static unsigned int highPrimes[10];
};

