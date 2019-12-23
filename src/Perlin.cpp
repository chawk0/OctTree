#include "Perlin.h"

unsigned int Perlin::randBase[10];
unsigned int Perlin::tinyPrimes[10] =
    {233, 17, 181, 23, 139, 43, 137, 47, 109, 83};
unsigned int Perlin::lowPrimes[10] =
    {15937, 15331, 15601, 15991, 15107, 15013, 15727, 15493, 15823, 15053};
unsigned int Perlin::midPrimes[10] =
    {789721, 789251, 789017, 789941, 789343, 789101, 789631, 789001, 789979, 789491};
unsigned int Perlin::highPrimes[10] =
    {1376312953, 1376312263, 1376312543, 1376312369, 1376312071,
     1376312881, 1376312501, 1376312989, 1376312017, 1376312507};

void Perlin::init()
{
	init((unsigned int)time(NULL));
}

void Perlin::init(unsigned int seed)
{
	srand(seed);
	for (int i = 0; i < 10; i++)
        randBase[i] = (unsigned int)rand();
}


double Perlin::PerlinNoise1D(double x, double frequency, double persistance)
{
    double f, a, r, scale;
    
    // The maximum amplitude of the sum of 8 noise curves, where the amplitude
    // of curve i is persistance^i, is a geometric series' sum.
    if (persistance < 1.0)
        scale = (1.0 - pow(persistance, 9.0)) / (1.0 - persistance);
    else
        scale = 8.0;

    r = 0.0;
    
    for (int octave = 0; octave < 8; octave++)
    {
        f = frequency * (double)(1 << octave);
        a = pow(persistance, (double)octave);
        r += (Perlin::cubicInterpNoise1D(x * f, octave) * a);
    }
    
    r /= scale;
    
    return r;
}

double Perlin::PerlinNoise2D(double x, double y, double frequency, double persistance)
{
    double f, a, r, scale;
    
    // The maximum amplitude of the sum of 8 noise curves, where the amplitude
    // of curve i is persistance^i, is a geometric series' sum.
    if (persistance < 1.0)
        scale = (1.0 - pow(persistance, 9.0)) / (1.0 - persistance);
    else
        scale = 8.0;

    r = 0.0;

    for (int octave = 0; octave < 8; octave++)
    {
        f = frequency * (double)(1 << octave);
        a = pow(persistance, (double)octave);
        r += (Perlin::cubicInterpNoise2D(x * f, y * f, octave) * a);
    }

    r /= scale;

    return r;
}

double Perlin::noise1D(unsigned int x, unsigned int n)
{
    x += randBase[n];
    x = (x << 13) ^ x;
    return (1.0 - ((x * (x * x * lowPrimes[n] + midPrimes[n]) + highPrimes[n]) & 0x7FFFFFFF) / 1073741824.0);
}

double Perlin::noise2D(unsigned int x, unsigned int y, unsigned int n)
{
    unsigned int t;

    t = randBase[n];
    t = t + x + y * tinyPrimes[n];
    t = (t << 13) ^ t;
    return (1.0 - ((t * (t * t * lowPrimes[n] + midPrimes[n]) + highPrimes[n]) & 0x7FFFFFFF) / 1073741824.0);
}

double Perlin::cubicInterpNoise1D(double x, unsigned int n)
{
    unsigned int intX = (unsigned int)x;
    double fracX = x - (double)intX;
    double p1, p2, p3, p4;
    
    p1 = noise1D(intX - 1, n);
    p2 = noise1D(intX, n);
    p3 = noise1D(intX + 1, n);
    p4 = noise1D(intX + 2, n);
    return cubicInterp(p1, p2, p3, p4, fracX);
}

double Perlin::cubicInterpNoise2D(double x, double y, unsigned int n)
{
    unsigned int intX = (unsigned int)x;
    unsigned int intY = (unsigned int)y;
    double fracX = x - (double)intX;
    double fracY = y - (double)intY;
    double p1, p2, p3, p4, m1, m2, m3, m4;
    
    p1 = noise2D(intX - 1, intY - 1, n);
    p2 = noise2D(intX, intY - 1, n);
    p3 = noise2D(intX + 1, intY - 1, n);
    p4 = noise2D(intX + 2, intY - 1, n);
    m1 = cubicInterp(p1, p2, p3, p4, fracX);
    
    p1 = noise2D(intX - 1, intY, n);
    p2 = noise2D(intX, intY, n);
    p3 = noise2D(intX + 1, intY, n);
    p4 = noise2D(intX + 2, intY, n);
    m2 = cubicInterp(p1, p2, p3, p4, fracX);
    
    p1 = noise2D(intX - 1, intY + 1, n);
    p2 = noise2D(intX, intY + 1, n);
    p3 = noise2D(intX + 1, intY + 1, n);
    p4 = noise2D(intX + 2, intY + 1, n);
    m3 = cubicInterp(p1, p2, p3, p4, fracX);
    
    p1 = noise2D(intX - 1, intY + 2, n);
    p2 = noise2D(intX, intY + 2, n);
    p3 = noise2D(intX + 1, intY + 2, n);
    p4 = noise2D(intX + 2, intY + 2, n);
    m4 = cubicInterp(p1, p2, p3, p4, fracX);
    
    return cubicInterp(m1, m2, m3, m4, fracY);
}

double Perlin::cubicInterp(double p1, double p2, double p3, double p4, double x)
{
    double T = (p1 - p2);
    double P = (p4 - p3) - T;
    double Q = T - P;
    double R = p3 - p1;
    double S = p2;
    
    double xSqr = x * x;
    double xCub = xSqr * x;
    
    return P * xCub + Q * xSqr + R * x + S;
}
