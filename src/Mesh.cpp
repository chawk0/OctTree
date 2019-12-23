#include "Mesh.h"

// load all object data from the provided file path;
// any currently loaded data is destroyed
void Mesh::load(const char* name, const char* fileName)
{
	ifstream fileIn;
	string token;

	fileIn.open(fileName, ios::in);
	if (fileIn.fail())
	{
		printf("error loading object file %s...\n", name);
		return;
	}
	else
	{
		destroy();

		Buffer newBuffer;
		string key, renderMode;
		int count, cc;

		_name = string(name);

		while (!fileIn.eof() && !fileIn.fail())
		{
			fileIn >> token;
			//printf("\ttoken len: %d, token: '%s'\n", token.length(), token.c_str());

			// beginning of a vertex attrib data stream
			if (token == "stream")
			{
				// the name of the stream is the key into the buffer map
				fileIn >> key;
				fileIn >> cc;
				fileIn >> count;

				// allocate space for and read into the buffer
				newBuffer.componentCount = cc;
				newBuffer.elementCount = count;
				newBuffer.len = count * cc * sizeof(float);
				newBuffer.data = new float[count * cc];
				// default value means this data stream is unused until 
				// it's set with StreamObject::setVertexAttribIndex
				newBuffer.index = 0xFFFFFFFF;
				
				for (int i = 0; i < count; ++i)
				{
					for (int j = 0; j < cc; ++j)
					{
						fileIn >> newBuffer.data[i * cc + j];
					}
				}

				// store in zee map!
				_buffers[key] = newBuffer;
			}
			// index data for indexed drawing
			else if (token == "index")
			{
				// index data includes a render mode, but it can specified separately for non-indexed drawing				
				fileIn >> renderMode;
				fileIn >> count;

				// default to unsigned shorts for now
				_iboData = new unsigned short[count];
				_iboDataType = GL_UNSIGNED_SHORT;
				_iboDataLen = count * sizeof(unsigned short);
				_elementCount = count;

				if (renderMode == "lines")
					_renderMode = GL_LINES;
				else if (renderMode == "triangles")
					_renderMode = GL_TRIANGLES;
				else if (renderMode == "tristrip")
					_renderMode = GL_TRIANGLE_STRIP;
				
				// read the data
				for (int i = 0; i < count; ++i)
					fileIn >> ((unsigned short*)_iboData)[i];

				_usingIBO = true;
			}
			// specify the render mode explicitly, typically when not using indexed drawing
			else if (token == "mode")
			{
				fileIn >> renderMode;

				if (renderMode == "lines")
					_renderMode = GL_LINES;
				else if (renderMode == "triangles")
					_renderMode = GL_TRIANGLES;
				else if (renderMode == "tristrip")
					_renderMode = GL_TRIANGLE_STRIP;
			}
			else if (token.length() > 0)
				printf("unknown token found: %s\n", token.c_str());

			// somehow I need this here
			token = "";
		}

		//if (_using
	}
}

// manually add a vertex data stream
void Mesh::addBuffer(const char* name, unsigned int index, int componentCount, int stride, float* data, unsigned int dataCount)
{
	Buffer newBuffer;
	unordered_map<string, Buffer>::iterator i = _buffers.find(string(name));

	// if there's already a buffer of that name, free the data first
	if (i != _buffers.end())
	{
		delete [] i->second.data;
	}
	else
	{
		// copy and store the provided data, instead of just copying the pointer
		newBuffer.index = index;
		newBuffer.data = new float[dataCount];
		newBuffer.componentCount = componentCount;
		newBuffer.len = dataCount * sizeof(float);
		// should be the same for all added buffers; checked during build()
		newBuffer.elementCount = dataCount / componentCount;
		// used in build() when concatenating all of the buffers into 1 VBO
		newBuffer.offset = 0;

		// if the provided data is tightly packed, then just memcpy
		if (stride == 0)
			memcpy_s(newBuffer.data, newBuffer.len, data, dataCount * sizeof(float));
		// otherwise, execute operation for-loop extraction
		else
		{
			unsigned char* src = (unsigned char*)data;
			for (int i = 0; i < (int)dataCount / componentCount; ++i)
			{
				// copy all of the floats 
				memcpy_s(&newBuffer.data[i * componentCount], componentCount * sizeof(float), src, componentCount * sizeof(float));
				src += stride;
			}
		}

		// add to map, indexed via the name as a std::string
		_buffers[string(name)] = newBuffer;
	}
}

Mesh::Buffer Mesh::getBufferData(const char* name)
{
	Buffer bd;
	memset(&bd, 0, sizeof(Buffer));

	// search for the stream by name
	auto i = _buffers.find(name);
	if (i != _buffers.end())
	{
		bd.data = i->second.data;
		bd.len = i->second.len;
		bd.componentCount = i->second.componentCount;
		bd.elementCount = i->second.elementCount;
	}

	return bd;
}

// manually add an index data stream
void Mesh::setIndexBuffer(GLenum renderMode, GLenum dataType, void* data, unsigned int dataCount)
{
	// if there's existing ibo data, free it
	if (_usingIBO)
	{
		delete [] _iboData;
		_iboData = NULL;
	}
	else
	{
		// set render mode and index element data type
		_renderMode = renderMode;
		_iboDataType = dataType;
		_elementCount = dataCount;

		// copy the data and compute size in bytes
		switch (_iboDataType)
		{
			case GL_UNSIGNED_SHORT:
				_iboData = new unsigned short[dataCount];
				_iboDataLen = dataCount * sizeof(unsigned short);
				memcpy_s(_iboData, _iboDataLen, data, dataCount * sizeof(unsigned short));
				break;

			case GL_UNSIGNED_INT:
				_iboData = new unsigned int[dataCount];
				_iboDataLen = dataCount * sizeof(unsigned int);
				memcpy_s(_iboData, _iboDataLen, data, dataCount * sizeof(unsigned int));
				break;
		};

		// yep
		_usingIBO = true;
	}
}

void Mesh::computeNormals(const char* streamName)
{
	Buffer newBuffer;
	float v[3];
	float* src;
	float* dst;
	unordered_map<string, Buffer>::iterator i;

	newBuffer.componentCount = 3;
	newBuffer.index = -1;
	newBuffer.offset = 0;

	// make sure the buffer exists
	i = _buffers.find(string(streamName));
	if (i != _buffers.end())
	{
		// fairly different process depending on if it's indexed data or not
		if (!_usingIBO)
		{
			switch (_renderMode)
			{
				case GL_TRIANGLES:
					newBuffer.data = new float[i->second.len / sizeof(float)];
					newBuffer.elementCount = i->second.elementCount;
					newBuffer.len = i->second.len;

					src = i->second.data;
					dst = newBuffer.data;
					
					// loop through the number of triangles
					for (int j = 0; j < (i->second.elementCount / 3); ++j)
					{
						// calculate normal
						_computeNormalFromTriangle(src, src + 3, src + 6, v);
						// store it 3 times, once for each vertex
						for (int k = 0; k < 3; ++k)
							memcpy_s(dst + k * 3, 3 * sizeof(float), v, 3 * sizeof(float));

						// 3 floats per vert, 3 verts per face
						src += 9;
						dst += 9;
					}

					_buffers["normal"] = newBuffer;
					break;
			}
		}
		else
		{
			// index-based normal computation!
		}
	}
	else
		printf("error computing normals, no vertex position stream available\n");
}

void Mesh::_computeNormalFromTriangle(float* in0, float* in1, float* in2, float* out)
{
	Vector3 v0 = Vector3(in0);
	Vector3 v1 = Vector3(in1);
	Vector3 v2 = Vector3(in2);
	Vector3 a = v1 - v0;
	Vector3 b = v2 - v0;
	Vector3 result = a.cross(b).normalize();

	out[0] = result.x;
	out[1] = result.y;
	out[2] = result.z;
}

void Mesh::build()
{
//#define CHECKGLERROR(func)	{int i = glGetError(); if (i) { Ef("[ERROR]: Mesh::build(): GL error %d in " #func " at line %d\n", i, __LINE__); }}
	// need at least 1 vertex attrib array
	if (_buffers.size() > 0)
	{
		glGetError();
		// generate and bind the VAO to hold everything
		glGenVertexArrays(1, &_vao);
		CHECKGLERROR(glGenVertexArrays)
		glBindVertexArray(_vao);
		CHECKGLERROR(glBindVertexArray)

		// there will be 1 VBO containing all of the vertex attrib arrays concatenated end-to-end
		glGenBuffers(1, &_vbo);
		CHECKGLERROR(glGenBuffers)
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		CHECKGLERROR(glBindBuffer)

		// compute size of final mega-buffer and allocate it
		unsigned int finalLen = 0;
		float* finalBuffer = NULL;
		
		for (unordered_map<string, Buffer>::iterator i = _buffers.begin(); i != _buffers.end(); ++i)
			finalLen += i->second.len;
		finalBuffer = new float[finalLen / sizeof(float)];

		// concatenatin'
		unsigned int currentOffset = 0;
		unsigned int elementCount = 0;
		bool mismatch = false;
		float* dst = NULL;

		dst = finalBuffer;
		for (unordered_map<string, Buffer>::iterator i = _buffers.begin(); i != _buffers.end(); ++i)
		{
			// for non-indexed drawing, elementCount is the number of n-tuples defining vertex attributes,
			// i.e. an elementCount of 5 means there're 5 position vec3's, 5 texture uv vec2's, etc;
			// each buffer should have the same number of elements, so check for that

			// the first buffer sets the count for the rest to match against
			if (i == _buffers.begin())
				elementCount = i->second.elementCount;
			else if (i->second.elementCount != elementCount)
			{
				mismatch = true;
				break;
			}

			// copy the current buffer's data into the current spot in the mega-buffer
			memcpy_s(dst, i->second.len, i->second.data, i->second.len);
			// save the offset in bytes so we can pass it to glVertexAttribPointer
			i->second.offset = currentOffset;
			// advance pointers
			dst += (i->second.len / sizeof(float));
			currentOffset += i->second.len;
		}

		if (mismatch)
		{
			printf("element count mismatch between vertex attrib data streams!\n");
			delete [] finalBuffer;
			destroy();
			return;
		}
		
		// upload
		glBufferData(GL_ARRAY_BUFFER, finalLen, finalBuffer, GL_STATIC_DRAW);
		CHECKGLERROR(glBufferData)

		// set the various vertex attrib properties
		for (unordered_map<string, Buffer>::iterator i = _buffers.begin(); i != _buffers.end(); ++i)
		{
			// make sure the data stream was set for use before enabling it
			if (i->second.index != 0xFFFFFFFF)
			{
				glEnableVertexAttribArray(i->second.index);
				CHECKGLERROR(glEnableVertexAttribArray)
				glVertexAttribPointer(i->second.index, i->second.componentCount, GL_FLOAT, GL_FALSE, 0, (GLvoid*)i->second.offset); 
				CHECKGLERROR(glVertexAttribPointer)
			}
		}
		
		// if indices are to be used, then setup the IBO
		if (_usingIBO)
		{
			glGenBuffers(1, &_ibo);
			CHECKGLERROR(glGenBuffers)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
			CHECKGLERROR(glBindBuffer)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, _iboDataLen, _iboData, GL_STATIC_DRAW);
			CHECKGLERROR(glBufferData)

			// in the case of indexed drawing, elementCount refers to the number of indices to render with
			switch (_iboDataType)
			{
				case GL_UNSIGNED_SHORT:
					_elementCount = _iboDataLen / sizeof(unsigned short);
					break;

				case GL_UNSIGNED_INT:
					_elementCount = _iboDataLen / sizeof(unsigned int);
					break;
			};
		}
		else
			// if not using indexed drawing, use the elementCount from the buffers, computed above
			_elementCount = elementCount;

		glBindVertexArray(0);
		CHECKGLERROR(glBindVertexArray)
	}
}

void Mesh::render()
{
	glBindVertexArray(_vao);
	if (_usingIBO)
		glDrawElements(_renderMode, _elementCount, _iboDataType, 0);
	else
		glDrawArrays(_renderMode, 0, _elementCount);
}

void Mesh::saveToFile(const char* fileName)
{
	ofstream fileOut;

	fileOut.open(fileName, ios::out);

	// if not using indexed drawing, output render mode explicitly
	if (!_usingIBO)
	{
		fileOut << "mode ";

		if (_renderMode == GL_LINES)
			fileOut << "lines" << endl;
		else if (_renderMode == GL_TRIANGLES)
			fileOut << "triangles" << endl;
		else if (_renderMode == GL_TRIANGLE_STRIP)
			fileOut << "tristrip" << endl;
		else
			fileOut << "null" << endl;
	}

	// output stream(s) data
	for (unordered_map<string, Buffer>::iterator i = _buffers.begin(); i != _buffers.end(); ++i)
	{
		// stream declaration is 'stream' <name> <component count per element> <total element count>
		fileOut << "stream " << i->first << " " << i->second.componentCount << " " << i->second.elementCount << endl;
		for (int j = 0; j < i->second.elementCount; ++j)
		{
			for (int k = 0; k < i->second.componentCount; ++k)
				fileOut << i->second.data[j * i->second.componentCount + k] << " ";
			fileOut << endl;
		}
		fileOut << endl;
	}

	// output index data, if used
	if (_usingIBO)
	{
		fileOut << "index ";

		// the number of indices per primitive
		int n;

		if (_renderMode == GL_LINES)
		{
			fileOut << "lines ";
			n = 2;
		}
		if (_renderMode == GL_TRIANGLES)
		{
			fileOut << "triangles ";
			n = 3;
		}
		else if (_renderMode == GL_TRIANGLE_STRIP)
		{
			fileOut << "tristrip ";
			n = 2;
		}

		fileOut << _elementCount << endl;

		// output 1 primitive's worth of indices per line
		for (int i = 0; i < (int)_elementCount; ++i)
		{
			fileOut << ((unsigned short*)(_iboData))[i] << " ";
			
			if (_renderMode == GL_TRIANGLE_STRIP)
			{
				// output first 3 for the initial triangle
				if (i == 2)
					fileOut << endl;
				else if (i > 2 && (i % 2) == 0)
					fileOut << endl;
			}
			// else just \n after each n values
			else if (((i + 1) % n) == 0)
				fileOut << endl;
		}
	}

	fileOut.close();
}

void Mesh::destroy()
{
	_name = "";
	_position = Vector3(0.0f, 0.0f, 0.0f);
	
	// delete VAO
	glDeleteVertexArrays(1, &_vao);
	_vao = 0;

	// delete VBO and associated data
	for (unordered_map<string, Buffer>::iterator i = _buffers.begin(); i != _buffers.end(); ++i)
		delete [] i->second.data;
	_buffers.clear();
	glDeleteBuffers(1, &_vbo);
	_vbo = 0;

	// delete IBO if used
	if (_usingIBO)
	{
		delete [] _iboData;
		glDeleteBuffers(1, &_ibo);
		_ibo = 0;
		_iboData = NULL;
		_iboDataLen = 0;
		_iboDataType = 0;
		_usingIBO = false;
	}

	// yadda yadda
	_elementCount = 0;
	_renderMode = 0;
}


void Mesh::setName(const char* name)
{
	_name = string(name);
}

void Mesh::setRenderMode(GLenum renderMode)
{
	_renderMode = renderMode;
}

// set the generic vertex attrib index of the given data stream, by name
void Mesh::setVertexAttribIndex(const char* buffer, unsigned int index)
{
	unordered_map<string, Buffer>::iterator i = _buffers.find(string(buffer));

	if (i != _buffers.end())
		i->second.index = index;
}
