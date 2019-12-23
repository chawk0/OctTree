#include "Font.h"

FT_Library* Font::_ftInstance = NULL;
uint Font::_ftInstanceRefCount = 0;


Font::Font():
	_glyphMap(NULL), _glyphData(NULL), _faceInfo(NULL), _color(1.0f, 1.0f, 1.0f)
{
	// share 1 static instance of the FT library across all Font objects
	if (_ftInstance == NULL)
	{
		_ftInstance = new FT_Library;
		FT_Init_FreeType(_ftInstance);
		// keep track of Font instances using it
		_ftInstanceRefCount = 1;
	}
	else
		_ftInstanceRefCount++;
}

Font::~Font()
{
	destroy();

	if (_ftInstanceRefCount == 1)
	{
		FT_Done_FreeType(*_ftInstance);
		_ftInstance = NULL;
	}
	else
		_ftInstanceRefCount--;
}

// loads a font .ttf file from disk, requests the given pixel size and builds the glyph map
void Font::load(const string& fontPath, int pixelFontSize)
{
	V("loading font...");

	// create font face
	_faceInfo = new FT_Face;
	FT_New_Face(*_ftInstance, fontPath.c_str(), 0, _faceInfo);

	// set size
	FT_Set_Pixel_Sizes(*_faceInfo, 0, pixelFontSize);
	//FT_Set_Char_Size(*_faceInfo, 0, (16 << 6), 0, 0);

	uint totalWidth = 0;
	uint totalHeight = 0;
	FT_Bitmap* b;

	// only 
	uint startIndex = 0x20;
	uint endIndex = 0x7E;

	// allocate data for each glyph
	_glyphData = new Glyph[256];

	// loop through glyphs and compute the total width of all of them in a row, and max height
	for (uint i = startIndex; i <= endIndex; i++)
	{
		FT_Load_Char(*_faceInfo, i, FT_LOAD_RENDER);
		b = &(*_faceInfo)->glyph->bitmap;
		totalWidth += b->width;
		
		// track largest glyph height
		if (b->rows > totalHeight)
			totalHeight = b->rows;		
	}

	// add 1 pixel padding between glyphs and between outer edges
	totalWidth += (endIndex - startIndex + 2);
	totalHeight += + 2;

	// create buffer to copy all the glyphs' pixels into
	Uint8* buffer = new Uint8[totalWidth * totalHeight];
	memset(buffer, 0, totalWidth * totalHeight);

	// track the starting position for the next glyph to be copied; Y is always 1
	uint glyphStartX = 1;
	uint srcIndex, dstIndex;

	// copy each glyph into the buffer and store the uv coords
	for (uint i = startIndex; i <= endIndex; i++)
	{
		FT_Load_Char(*_faceInfo, i, FT_LOAD_RENDER);
		b = &((*_faceInfo)->glyph->bitmap); // wow.

		// save relevant glyph data for rendering
		_glyphData[i].val = i;

		_glyphData[i].bearingX = (*_faceInfo)->glyph->bitmap_left;
		_glyphData[i].bearingY = (*_faceInfo)->glyph->bitmap_top;

		_glyphData[i].advance = (*_faceInfo)->glyph->advance.x;

		//Vf("glyph '%c': size: %dx%d, bearingX: %d, bearingY: %d, advance: %d", i, b->width, b->rows, _glyphData[i].bearingX, _glyphData[i].bearingY, _glyphData[i].advance);

		// calculate the 4 corners of the font glyph in uv space, also accounting for the 1 pixel border
		float u0 = static_cast<float>(glyphStartX) / static_cast<float>(totalWidth);
		float u1 = static_cast<float>(glyphStartX + b->width) / static_cast<float>(totalWidth);
		float v0 = 1.0f / static_cast<float>(totalHeight);
		float v1 = static_cast<float>(1 + b->rows) / static_cast<float>(totalHeight);

		//Vf("glyph '%c': u %.5f %.5f, v %.5f %.5f", i, u0, u1, v0, v1);

		_glyphData[i].width = b->width;
		_glyphData[i].height = b->rows;
		
		// this holds the sequence of 12 uv coords for the 6 elements making 2 gl_triangles
		_glyphData[i].uv[0] = u0; _glyphData[i].uv[1] = v0;
		_glyphData[i].uv[2] = u0; _glyphData[i].uv[3] = v1;
		_glyphData[i].uv[4] = u1; _glyphData[i].uv[5] = v0;
		_glyphData[i].uv[6] = u1; _glyphData[i].uv[7] = v0;
		_glyphData[i].uv[8] = u0; _glyphData[i].uv[9] = v1;
		_glyphData[i].uv[10] = u1; _glyphData[i].uv[11] = v1;

		// loop through glyph buffer and copy to bigger buffer
		for (uint y = 0; y < b->rows; y++)
		{
			for (uint x = 0; x < b->width; x++)
			{
				srcIndex = x + y * b->width;
				// offset into buffer using glyphStartX
				dstIndex = glyphStartX + x + (y + 1) * totalWidth;
				buffer[dstIndex] = b->buffer[srcIndex];
			}
		}
		
		// shift glyphstart for next glyph copy
		glyphStartX += (b->width + 1);
	}

	// now create a GL texture from the buffer
	_glyphMap = new Texture();
	_glyphMap->create(buffer, static_cast<int>(totalWidth), static_cast<int>(totalHeight), GL_RED, TCF_TEXTURE);
	delete [] buffer;

	// setup the VBO and other render data for the 
	
	// start with vertex attributes
	_renderData.vertexAttribs.alloc(1);

	// combine 2d font pos and 2d uv coords into 1 vec4 and thus 1 vert attrib
	VertexAttrib va;
	va.name = "pos_uv";
	va.index = 0;
	va.componentCount = 4;
	va.dataType = GL_FLOAT;
	va.stride = 0;
	va.offset = 0;
	_renderData.vertexAttribs[0] = va;

	// create the VAO to hold all the bindings and settings
	glGenVertexArrays(1, &_renderData.vao);
	CHECKGLERROR(glGenVertexArrays)
	glBindVertexArray(_renderData.vao);
	CHECKGLERROR(glBindVertexArray)

	// create the VBO
	_renderData.vertexBuffers.alloc(1);
	
	VertexBuffer vb;
	vb.target = GL_ARRAY_BUFFER;
	vb.usage = GL_DYNAMIC_DRAW;
	// allocate space for 2000 characters, represented as quads (2 sequential GL_TRIANGLES primitives);
	// 6 elements total, 4 floats per element
	vb.size = 2000 * 6 * 4 * sizeof(float);
	// use the client side buffer to render font strings into before updating the VBO
	vb.data = new float[vb.size / sizeof(float)];
	memset(vb.data, 0, vb.size);
	/*
	vb.data[0] = 0.0f; vb.data[1] = 0.0f;
	vb.data[2] = 0.0f; vb.data[3] = 0.0f; 
	
	vb.data[4] = 0.0f; vb.data[5] = 256.0f; 
	vb.data[6] = 0.0f; vb.data[7] = 1.0f; 
	
	vb.data[8] = 256.0f; vb.data[9] = 0.0f; 
	vb.data[10] = 1.0f; vb.data[11] = 0.0f; 
	*/

	glGenBuffers(1, &vb.vbo);
	CHECKGLERROR(glGenBuffers)

	_renderData.vertexBuffers[0] = vb;
		
	glBindBuffer(_renderData.vertexBuffers[0].target, _renderData.vertexBuffers[0].vbo);
	CHECKGLERROR(glBindBuffer)
	glBufferData(_renderData.vertexBuffers[0].target, _renderData.vertexBuffers[0].size, _renderData.vertexBuffers[0].data, _renderData.vertexBuffers[0].usage);
	CHECKGLERROR(glBufferData)
	
	// setup pos and uv interleaved into 1 vec4 vert attrib, (x0|y0|u0|v0, x1|y1|u1|v1, ...)
	glEnableVertexAttribArray(_renderData.vertexAttribs[0].index);
	glVertexAttribPointer(_renderData.vertexAttribs[0].index, _renderData.vertexAttribs[0].componentCount, _renderData.vertexAttribs[0].dataType, GL_FALSE, _renderData.vertexAttribs[0].stride, _renderData.vertexAttribs[0].offset);
	CHECKGLERROR(glVertexAttribPointer)

	/*glEnableVertexAttribArray(_renderData.vertexAttribs[1].index);
	glVertexAttribPointer(_renderData.vertexAttribs[1].index, _renderData.vertexAttribs[1].componentCount, _renderData.vertexAttribs[1].dataType, GL_FALSE, _renderData.vertexAttribs[1].stride, _renderData.vertexAttribs[1].offset);
	CHECKGLERROR(glVertexAttribPointer)*/
	
	glBindVertexArray(0);

	// lastly, allocate a client-side buffer to hold temporary pos/uv data to update the VBO with during font rendering
	//_renderData.vertexBuffers[0].data = new float[2000 * 6 * 4];
	assert(_renderData.vertexBuffers[0].data != nullptr);
}

void Font::printf(float x, float y, const string& format, ...)
{
	char output[2001];
	va_list args;

	va_start(args, format);	
	int result = vsnprintf_s(output, 2001, 2000, format.c_str(), args);
	va_end(args);

	print(x, y, output);
}

void Font::print(float x, float y, const string& text)
{
	float startX = x;
	float startY = y;
	float gx, gy;
	float pos[12];
	uint posPtr, uvPtr;
	float* data = _renderData.vertexBuffers[0].data;

	// setup the pointers into the temp buffer for writing pos and uv data
	posPtr = 0; uvPtr = 2;

	// loop through each character of the resulting string
	for (int i = 0; i < static_cast<int>(text.length()); i++)
	{
		// copy the glyph into the temp client-side buffer
		Glyph* g = &_glyphData[text[i]];

		startX += static_cast<float>(g->bearingX);
		startY = y - static_cast<float>(g->bearingY);

		if (g->val < 0x20 || g->val > 0x7E)
		{
			cout << "bad glyph!" << endl;
			continue;
		}
		
		gx = static_cast<float>(g->width);
		gy = static_cast<float>(g->height);

		// generate coords from the x,y offset and the glyph sizes
		pos[0]  = startX;      pos[1]  = startY;
		pos[2]  = startX;      pos[3]  = startY + gy;
		pos[4]  = startX + gx; pos[5]  = startY;
		pos[6]  = startX + gx; pos[7]  = startY;
		pos[8]  = startX;      pos[9]  = startY + gy;
		pos[10] = startX + gx; pos[11] = startY + gy;

		// copy the pos and uv data, interleaved, into the temp buffer
		// 6 elements
		for (uint j = 0; j < 6; j++)
		{
			data[posPtr] = pos[j * 2 + 0]; data[posPtr + 1] = pos[j * 2 + 1];
			data[uvPtr] = g->uv[j * 2 + 0]; data[uvPtr + 1] = g->uv[j * 2 + 1];
			posPtr += 4;
			uvPtr += 4;
		}

		// advance by (glyph->advance / 64) pixels
		startX += (static_cast<float>(g->advance) / 64.0f - static_cast<float>(g->bearingX));
	}
	

	// update the VBO
	glBindBuffer(_renderData.vertexBuffers[0].target, _renderData.vertexBuffers[0].vbo);
	CHECKGLERROR(glBindBuffer1)
	glBufferSubData(_renderData.vertexBuffers[0].target, 0, uvPtr * sizeof(float), data);
	CHECKGLERROR(glBufferSubData)
	glBindBuffer(_renderData.vertexBuffers[0].target, 0);
	CHECKGLERROR(glBindBuffer2)
	
	// render the shit
	/*shader->setUniform1i("useTexturing", 1);
	shader->setUniform1i("useHeightFactor", 0);
	shader->setMatrix4f("WVPMatrix", testMatrix);*/
	glVertexAttrib4f(1, _color.x, _color.y, _color.z, 1.0f);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	_glyphMap->bind(0);
	glBindVertexArray(_renderData.vao);
	CHECKGLERROR(glBindVertexArray)

	glDrawArrays(GL_TRIANGLES, 0, uvPtr / 4);
	CHECKGLERROR(glDrawArrays)

	glBindVertexArray(0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	//shader->setUniform1i("useTexturing", 0);
}

void Font::setColor(float r, float g, float b)
{
	_color.x = r;
	_color.y = g;
	_color.z = b;
}

void Font::destroy()
{
	if (_glyphMap != NULL)
	{
		delete _glyphMap;
		_glyphMap = NULL;
	}

	if (_glyphData != NULL)
	{
		delete [] _glyphData;
		_glyphData = NULL;
	}

	if (_faceInfo != NULL)
	{
		FT_Done_Face(*_faceInfo);
		delete _faceInfo;
		_faceInfo = NULL;
	}
}

Texture* Font::getTexture()
{
	return _glyphMap;
}

uint Font::getTextureWidth()
{
	return _glyphMap->getWidth();
}

uint Font::getTextureHeight()
{
	return _glyphMap->getHeight();
}