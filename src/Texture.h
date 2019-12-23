#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
//#include "Camera.h"
//#include "Engine.h"
#include "Log.h"
//#include "MatrixStack.h"
//#include "Perlin.h"
//#include "SceneObject.h"
//#include "Shader.h"
//#include "Sketch.h"
//#include "Timer.h"
//#include "World.h"

// creation flags
#define TCF_TEXTURE     0x00000001
#define TCF_BUFFERED    0x00000002
#define TCF_BOTH        TCF_TEXTURE | TCF_BUFFERED
#define TCF_MIPMAP      0x00000004
#define TCF_FORCE_32    0x00000008

// texture file types for getFileType()
#define TFT_BMP 1
#define TFT_TGA 2
#define TFT_PNG 3
#define TFT_JPG 4
#define TFT_PCX 5
#define TFT_GIF 6

#ifndef GL_BGR
	#define GL_BGR		0x80E0
#endif
#ifndef GL_BGRA
	#define GL_BGRA		0x80E1
#endif

//------------------------------------------------------------------------------

class Texture
{
    public:
        Texture()
        {
            _width = 0; _height = 0;
            _minFilter = GL_LINEAR; _magFilter = GL_LINEAR;
            _isCreated = false, _ownsBuffer = false;
            _id = 0; _target = 0;
			_buffer = NULL; _bufferPixelSize = 0; _bufferPixelFormat = 0;
        }
        ~Texture()
        {
			destroy();
        }
        
        int create(const char* file);
        int create(const char* file, int minFilter, int magFilter, Uint32 flags);
        int create(Uint8* buffer, int width, int height, int pixelFormat, Uint32 flags);
		int resample(int pixelSize, int pixelFormat);
        void destroyBuffer();
		void destroyTexture();
		void destroy()
		{
			destroyTexture();
			destroyBuffer();
		}
		void bind();
		void bind(GLenum unit);
		void unbind();
		void unbind(GLenum unit);
        
        bool isCreated() { return _isCreated; }
        bool hasBuffer() { return (_buffer != NULL); }
        
        int getWidth() { return _width; }
        int getHeight() { return _height; }
        Uint8* getBuffer() { return _buffer; }
        Uint32 getID() { return _id; }

        void setMinFilter(int minFilter);
        void setMagFilter(int magFilter);
        
    private:
		int loadToBuffer(const char* file);
		int _finalize(int minFilter, int magFilter, Uint32 flags);
        static int getFileType(const char* file);
		static void byteSwap(Uint8* buffer, int pixelCount, int skip);
        
        int _width, _height;
        int _minFilter, _magFilter;
        bool _isCreated, _ownsBuffer;
        Uint32 _id, _target;
        Uint8* _buffer;
		int _bufferPixelSize, _bufferPixelFormat;
};


