#include "Texture.h"

/*
	creates a texture from file with default filters (GL_LINEAR) and builds
	mipmaps
*/
int Texture::create(const char* file)
{
    return create(file, 0, 0, TCF_TEXTURE | TCF_MIPMAP);
}

/*
	loads a texture from file and creates either a GL texture, a buffer, or
	both based on given flags
*/
int Texture::create(const char* file, int minFilter, int magFilter, Uint32 flags)
{
	V("[Texture]: texture creation request");
    if (_isCreated)
    {
        V("[Texture]: destroying old texture...");
        destroy();
    }
    _isCreated = false;

	if (!file)
	{
		E("[ERROR]: Texture::create(): bad args; file == NULL");
		destroy();
		//_error = TEXTURE_ERROR_BAD_ARGS;
		return -1;
	}

    Vf("[Texture]: opening %s...", file);
    ifstream fileIn;
    fileIn.open(file, ios::in | ios::binary);
    if (fileIn.fail())
    {
        Ef("[ERROR]: Texture::create(): failed to open %s", file);
		destroy();
        //_error = TEXTURE_ERROR_BAD_FILE;
        return -1;
    }
    fileIn.close();

	// load the image data into the buffer
	V("[Texture]: loading image data...");
	if (loadToBuffer(file) != 0)
	{
		E("[ERROR]: Texture::create(): error loading image data");
		destroy();
		return -1;
	}
	Vf("\t%dx%d, pixel size: %d", _width, _height, _bufferPixelSize * 8);

	_finalize(minFilter, magFilter, flags);

	return 0;
}

int Texture::create(Uint8* buffer, int width, int height, int pixelFormat, Uint32 flags)
{
	V("[Texture]: texture creation request");
    if (_isCreated)
    {
        V("[Texture]: destroying old texture...");
        destroy();
    }
    _isCreated = false;

	if (buffer == NULL || width == 0 || height == 0)
	{
		E("[ERROR]: Texture::create(): bad buffer parameters given");
		return -1;
	}
	else
	{
		/*
		 int _width, _height;
        int _minFilter, _magFilter;
        bool _isCreated;
        Uint32 _id, _target;
        Uint8* _buffer;
		int _bufferPixelSize, _bufferPixelFormat;
		*/

		_ownsBuffer = false;
		_buffer = buffer;
		_width = width;
		_height = height;
		switch (pixelFormat)
		{
		case GL_RED:
			_bufferPixelSize = 1;
			break;
		case GL_RGB:
		case GL_BGR:
			_bufferPixelSize = 3;
			break;
		case GL_RGBA:
		case GL_BGRA:
			_bufferPixelSize = 4;
			break;

		default:
			break;
		};
			
		_bufferPixelFormat = pixelFormat;

		Vf("\t%dx%d, pixel size: %d", _width, _height, _bufferPixelSize * 8);

		_finalize(0, 0, flags);

		return 0;
	}
}

int Texture::_finalize(int minFilter, int magFilter, Uint32 flags)
{
	/* um what?
	if (getFileType(file) == TFT_PNG)
	{
		for (int i = 0; i < (_width * _height); ++i)
			_buffer[i * 4 + 3] = 0x40;
	}
	*/

	// resample data to 32bit color if requested
	if (flags & TCF_FORCE_32)// || _bufferPixelSize == 4)
	{
		V("[Texture]: (re)sampling...");
		int result = resample(4, GL_RGBA);
		if (result != 0)
		{
			E("[ERROR]: Texture::create(): error resampling image data");
			destroy();
			return -1;
		}
	}
	//else
	//	result = resample(3, GL_RGB);
	
	

	// build texture and/or assign buffer ptr
    if (minFilter == 0)
    {
        if (flags & TCF_MIPMAP)
            _minFilter = GL_LINEAR_MIPMAP_LINEAR;
        else
            _minFilter = GL_LINEAR;
    }
    else
        _minFilter = minFilter;
    
    if (magFilter == 0)
        _magFilter = GL_LINEAR;
    else
        _magFilter = magFilter;

	_target = GL_TEXTURE_2D;

//#define CHECKGLERROR(func)	{int i = glGetError(); if (i) { Ef("[ERROR]: Texture::create(): GL error %d at " #func, i); }}

    if (flags & TCF_TEXTURE)
    {
		glGetError();
		V("[Texture]: generating GL texture...");
        glGenTextures(1, &_id);
		CHECKGLERROR(glGenTextures)
		Vf("\tid: %d", _id);
		V("[Texture]: binding...");
        glBindTexture(_target, _id);
		CHECKGLERROR(glBindTexture)
			
		switch (_target)
		{
			case GL_TEXTURE_2D:
				V("[Texture]: loading pixels...");
				glTexImage2D(_target, 0, _bufferPixelFormat, _width, _height, 0,
					_bufferPixelFormat, GL_UNSIGNED_BYTE, _buffer);
				CHECKGLERROR(glTexImage2D)
				break;

			default:
				E("[ERROR]: Texture::create(): unsupported texture target");
				destroy();
				return -1;
		}

		V("[Texture]: settings tex params...");
		glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		CHECKGLERROR(glTexParameteri)
            
        if (flags & TCF_MIPMAP)
		{
			V("[Texture]: building mipmaps...");
			glGenerateMipmap(_target);
			CHECKGLERROR(glGenerateMipmap)
			V("[Texture]: settings tex params...");
			glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	// tri-linear!
			CHECKGLERROR(glTexParameteri)
		}
        else
		{
			glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			CHECKGLERROR(glTexParameteri)
		}
        
        if (!glGetError())
			_isCreated = true;
        else
        {
			E("[ERROR]: Texture::create(): failed to create GL texture");
            destroy();
			return -1;
        }
    }
    
    if (flags & TCF_BUFFERED)
	{
		V("[Texture]: saving buffer data...");
	}
	else
		destroyBuffer();

    V("[Texture]: done.");

//#undef CHECKGLERROR

	return 0;
}

int Texture::loadToBuffer(const char* file)
{
	int fileType = getFileType(file);
    if (fileType == 0)
    {
        Ef("[ERROR]: Texture::loadToBuffer(): unsupported file extension for %s", file);
		destroy();
        //_error = TEXTURE_ERROR_BAD_FILE;
		return -1;
    }
    
    SDL_Surface* s;
    SDL_RWops* rwops;
    
    rwops = SDL_RWFromFile(file, "rb");
    if (!rwops)
    {
        Ef("[ERROR]: Texture::loadToBuffer(): failed to get RWops for %s", file);
		destroy();
        //_error = TEXTURE_ERROR_BAD_FILE;
        return -1;
    }
    
    switch (fileType)
    {
        case TFT_BMP:
            s = IMG_LoadBMP_RW(rwops); break;
        case TFT_TGA:
            s = IMG_LoadTGA_RW(rwops); break;
        case TFT_PNG:
            s = IMG_LoadPNG_RW(rwops); break;
        case TFT_JPG:
            s = IMG_LoadJPG_RW(rwops); break;
        case TFT_PCX:
            s = IMG_LoadPCX_RW(rwops); break;
        case TFT_GIF:
            s = IMG_LoadGIF_RW(rwops); break;
    }
    
    if (!s)
    {
        Ef("[ERROR]: Texture::loadToBuffer(): failed IMG_Load for %s", file);
		destroy();
		return -1;
    }

	if (s->format->BitsPerPixel != 24 && s->format->BitsPerPixel != 32)
	{
		Ef("[ERROR]: Texture::loadToBuffer(): unsupported pixel size (%d)", s->format->BitsPerPixel);
		destroy();
		return -1;
	}
	_bufferPixelSize = s->format->BytesPerPixel;

	int bufferSize = s->w * s->h * s->format->BytesPerPixel;
	_buffer = new (nothrow) Uint8[bufferSize];
	if (!_buffer)
	{
		E("[ERROR]: Texture::loadToBuffer(): failed to allocate buffer memory");
		destroy();
		return -1;
	}
	_ownsBuffer = true;
	memset(_buffer, 0, bufferSize);
	
	Uint8* pixels = (Uint8*)s->pixels;

	// vertically flip
	for (int y = 0; y < s->h; y++)
		memcpy(&_buffer[y * (int)s->pitch], &pixels[(s->h - y - 1) * (int)s->pitch], s->pitch);

	if (s->format->BitsPerPixel == 24 && (fileType == TFT_BMP || fileType == TFT_TGA))
		_bufferPixelFormat = GL_BGR;
	else if (s->format->BitsPerPixel == 32 && fileType == TFT_TGA)
		_bufferPixelFormat = GL_BGRA;
	else if (_bufferPixelSize == 3)
		_bufferPixelFormat = GL_RGB;
	else
		_bufferPixelFormat = GL_RGBA;

	_width = s->w;
	_height = s->h;

	SDL_FreeSurface(s);

	return 0;
}

int Texture::resample(int pixelSize, int pixelFormat)
{
	if (!hasBuffer())
		return -1;

	// already sampled to desired format?
	if (_bufferPixelSize == pixelSize && _bufferPixelFormat == pixelFormat)
		return 0;

	// don't allow pixel size of 3 with GL_RGBA/GL_BGRA nor
	// pixel size of 4 with GL_RGB/GL_BGR
	if (((pixelSize == 3) && ((pixelFormat == GL_RGBA) || (pixelFormat == GL_BGRA))) ||
		((pixelSize == 4) && ((pixelFormat == GL_RGB) || (pixelFormat == GL_BGR))))
		return -1;

	Uint8* newBuffer;
	int newBufferSize;
	int srcPtr, destPtr;

	// pixelSize is the destination pixel size
	switch (pixelSize)
	{
		// 24 bit
		case 3:
		{
			// already 24 bit? check formats and swap if needed
			// only GL_RGB and GL_BGR are supported
			if ((_bufferPixelSize == 3) && (_bufferPixelFormat != pixelFormat))
			{
				byteSwap(_buffer, _width * _height, 0);
				_bufferPixelFormat = pixelFormat;
			}
			// else, discard A and copy
			else if (_bufferPixelSize == 4)
			{
				newBufferSize = _width * _height * pixelSize;
				newBuffer = new (nothrow) Uint8[newBufferSize];
				if (!newBuffer)
				{
					E("[ERROR]: Texture::resample(): failed to allocate new buffer memory");
					destroy();
					return -1;
				}
				memset(newBuffer, 0, newBufferSize);

				srcPtr = 0; destPtr = 0;
				for (int i = 0; i < (_width * _height); i++)
				{
					newBuffer[destPtr++] = _buffer[srcPtr++];
					newBuffer[destPtr++] = _buffer[srcPtr++];
					newBuffer[destPtr++] = _buffer[srcPtr++];
					srcPtr++;
				}

				delete [] _buffer;
				_buffer = newBuffer;
				_bufferPixelSize = 3;
				// update pixel format
				if (_bufferPixelFormat == GL_RGBA)
					_bufferPixelFormat = GL_RGB;
				else if (_bufferPixelFormat == GL_BGRA)
					_bufferPixelFormat = GL_BGR;

				// now compare with target pixel format and swap if needed
				if (_bufferPixelFormat != pixelFormat)
				{
					byteSwap(_buffer, _width * _height, 0);
					_bufferPixelFormat = pixelFormat;
				}
			}
		}
		break;

		case 4:
		{
			// if current buffer is 24 bit, add an alpha channel
			if (_bufferPixelSize == 3)
			{
				// create A
				newBufferSize = _width * _height * pixelSize;
				newBuffer = new (nothrow) Uint8[newBufferSize];
				if (!newBuffer)
				{
					E("[ERROR]: Texture::resample(): failed to allocate new buffer memory");
					destroy();
					return -1;
				}
				memset(newBuffer, 0, newBufferSize);
				
				srcPtr = 0; destPtr = 0;
				for (int i = 0; i < (_width * _height); i++)
				{
					newBuffer[destPtr++] = _buffer[srcPtr++];
					newBuffer[destPtr++] = _buffer[srcPtr++];
					newBuffer[destPtr++] = _buffer[srcPtr++];
					newBuffer[destPtr++] = 255;
				}

				delete [] _buffer;
				_buffer = newBuffer;
				_bufferPixelSize = 4;
				// update pixel format
				if (_bufferPixelFormat == GL_RGB)
					_bufferPixelFormat = GL_RGBA;
				else if (_bufferPixelFormat == GL_BGR)
					_bufferPixelFormat = GL_BGRA;

				// check format
				if (_bufferPixelFormat != pixelFormat)
				{
					byteSwap(_buffer, _width * _height, 1);
					_bufferPixelFormat = pixelFormat;
				}
			}
			else if ((_bufferPixelSize == 4) && (_bufferPixelFormat != pixelFormat))
			{
				byteSwap(_buffer, _width * _height, 1);
				_bufferPixelFormat = pixelFormat;
			}
		}
		break;

		default:
			E("[ERROR]: Texture::resample(): invalid destination pixel size");
			destroy();
			return -1;
	};

	return 0;
}

int Texture::getFileType(const char* file)
{
    char extension[4];
    int len;
    Uint32 e;

    if (file)
    {
        len = (int)strlen(file);
        // Copy to extension and lowercase'ize
        for (int i = 0; i < 3; i++)
            extension[i] = file[len - 3 + i] | 0x20;
        extension[3] = 0;

        e = *((Uint32*)extension);
        switch (e)
        {
            case 0x00706D62: return TFT_BMP;
            case 0x00616774: return TFT_TGA;
            case 0x00676E70: return TFT_PNG;
            case 0x0067706A: return TFT_JPG;
            case 0x00786370: return TFT_PCX;
            case 0x00666967: return TFT_GIF;
            default: return 0;
        }
    }
	else
		return 0;
}

void Texture::destroyTexture()
{
	if (_id) glDeleteTextures(1, &_id);
    _id = 0;
    _isCreated = false;
	_width = 0; _height = 0;
	_minFilter = 0; _magFilter = 0;
}

void Texture::destroyBuffer()
{
	if (_buffer && _ownsBuffer) delete [] _buffer;
	_buffer = NULL;
	_bufferPixelSize = 0; _bufferPixelFormat = 0;
}

void Texture::byteSwap(Uint8* buffer, int pixelCount, int skip)
{
	// only RGB <-> BGR swapping is supported
	// buffer is assumed to be (3 + skip) * pixelCount bytes in size
	
	int ptr = 0;
	for (int i = 0; i < pixelCount; i++)
	{
		buffer[ptr + 0] ^= buffer[ptr + 2];
		buffer[ptr + 2] ^= buffer[ptr + 0];
		buffer[ptr + 0] ^= buffer[ptr + 2];
		ptr = ptr + 3 + skip;
	}
}

void Texture::bind()
{
	glBindTexture(_target, _id);
}

void Texture::bind(GLenum unit)
{
	glActiveTexture(unit + GL_TEXTURE0);
	bind();
}

void Texture::unbind()
{
	glBindTexture(_target, 0);
}

void Texture::unbind(GLenum unit)
{
	glActiveTexture(unit + GL_TEXTURE0);
	glBindTexture(_target, 0);
}
