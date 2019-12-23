/*
	Font rendering classed based on the FreeType library.  Renders to a dynamic VBO
*/

#pragma once
#include "ChawkFortress.h"
#include "Texture.h"

class Font
{
	public:
		Font();
		~Font();

		void load(const string& fontPath, int pixelFontSize);
		void destroy();

		void setColor(float r, float g, float b);

		void print(float x, float y, const string& text);
		void printf(float x, float y, const string& format, ...);

		Texture* getTexture();
		uint getTextureWidth();
		uint getTextureHeight();

	private:
		struct Glyph
		{
			Uint8 val;
			int bearingX, bearingY;
			int advance;
			uint width, height;
			float uv[12];
		};

		// no copy constructor
		Font(const Font&);

		// FT_Face holds all of the nitty gritty details of the font
		FT_Face* _faceInfo;
		// the GL texture object that we use in rendering
		Texture* _glyphMap;
		// !
		Glyph* _glyphData;
		//
		Vector3 _color;

		// holds lists of vertex data used each frame to update the main VBO
		//float* _tempBuffer;
		RenderData _renderData;

		// share 1 static instance of the FT library across all Font objects
		static FT_Library* _ftInstance;
		static uint _ftInstanceRefCount;
};