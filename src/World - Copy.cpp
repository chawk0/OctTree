#include "World.h"


World::World():
		_grid(NULL),
		_width(0),
		_height(0),
		_depth(0)
{
	//
}

World::~World()
{
	// free the world grid
	if (_grid != NULL)
		delete [] _grid;

	// free each texture of each voxel def
	for (vector<Voxel>::iterator i = _voxelDefs.begin(); i != _voxelDefs.end(); ++i)
		if (i->texture != NULL)
			delete i->texture;

	// delete the VAO, VBO, and buffers for each segment
	for (auto i = _segments.begin(); i != _segments.end(); ++i)
	{
		glDeleteVertexArrays(1, &i->vao);
		delete [] i->buffer.data;
		glDeleteBuffers(1, &i->vbo);
	}
};

void World::create(int width, int height, int depth)
{
	try
	{
		if (_grid != NULL)
			delete [] _grid;

		_grid = new int[width * height * depth];
		_width = width;
		_height = height;
		_depth = depth;

		// id 0 is empty
		memset(_grid, 0, sizeof(int) * _width * _height * _depth);

		// load voxel defs
		_loadVoxelDefs();
		// populate world grid with Perlin noise to form a landscape, including water
		_generateLandscape();
		// compute all visible faces and build a render buffer
		_buildRenderBuffer();

		_object.load("cube", "./res/objects/cube_n.obj");
		
	}
	catch (std::bad_alloc& info)
	{
		cout << "failed to allocate Voxel data (" << info.what() << ")" << endl;
	}
	catch (exception e)
	{
		cout << "unknown error in World create (" << e.what() << ")" << endl;
	}
}

void World::_loadVoxelDefs()
{
	ifstream fileIn;
	string token;
	char buffer[261];
	Voxel newVoxel;
	// texture pointer defaults to NULL
	newVoxel.texture = NULL;

	cout << "loading voxel defs..." << endl;
	try
	{
		// load the voxel definitions
		fileIn.open("./res/defs/voxels.txt", ios::in);

		while (!fileIn.eof() && !fileIn.fail())
		{
			fileIn >> token;

			if (token.length() > 0)
			{
				// start of a voxel def.
				// expecting "id <n>", followed by various other attribute defs
				if (token == "id")
				{
					fileIn >> newVoxel.id;
					cout << "reading voxel id: " << newVoxel.id << endl;
				}
				else if (token == "name")
				{
					fileIn >> newVoxel.name;
					cout << "\tvoxel name: " << newVoxel.name << endl;
				}
				else if (token == "tex")
				{
					// max OS file path is around 260 characters
					fileIn.getline(buffer, 260);
					token = string(buffer);
					cout << "\tvoxel texture path: " << token << endl;
					
					// trim leading and trailing whitespace
					auto first = token.find_first_not_of(" \t");
					auto len = token.find_last_not_of(" \t") - first + 1;
					token = token.substr(first, len);

					// create the texture object
					newVoxel.texture = new Texture();
					newVoxel.texture->create(token.c_str(), 0, 0, TCF_TEXTURE | TCF_MIPMAP | TCF_FORCE_32);
				}
				// once this token is reached, the current state of newVoxel is added as a new Voxel def to the vector
				else if (token == "end")
				{
					_voxelDefs.push_back(newVoxel);
					// ensure each new voxel def has a NULL texture pointer by default. makes cleanup easier
					newVoxel.texture = NULL;
				}
				else
					cout << "unknown token in voxel def: " << token << endl;
			}
		}
	}
	catch (exception e)
	{
		cout << "exception loading voxel defs (" << e.what() << ")" << endl;
	}
}

void World::_generateLandscape()
{
	
	// fill with water up to a certain height (35% the full range of height)
	int waterHeight = static_cast<int>(static_cast<float>(_height) * 0.35f);

	if (waterHeight > 0)
	{
		// loop over the XZ plane and fill columns of voxels
		for (int x = 0; x < _width; ++x)
			for (int z = 0; z < _depth; ++z)
				for (int y = 0; y < waterHeight; ++y)
				{
					int index = x + y * _width + z * _width * _height;
					_grid[index] = 2;
				}
	}
	
	
	// generate random perlin noise in the XZ direction, with the noise value becoming terrain height
	for (int x = 0; x < _width; ++x)
	{
		for (int z = 0; z < _depth; ++z)
		{
			// the Perlin noise functions return doubles between [-1.0, 1.0] but the distribution varies with persistence.
			// bias the value to [0.0, 1.0] then scale by grid height, clamping afterwards
			int maxY = static_cast<int>((Perlin::PerlinNoise2D(static_cast<double>(x), static_cast<double>(z), 0.005, 0.50) / 2.0 + 0.5) * static_cast<double>(_height));
			if (maxY < 0)
				maxY = 0;
			else if (maxY >= _height)
				maxY = _height;

			// fill the column of dirt up to maxY
			for (int y = 0; y < maxY; ++y)
			{
				int index = x + y * _width + z * _width * _height;
				_grid[index] = 1;
			}			
		}
	}
	
	/*
	// generate flat terrain
	for (int x = 0; x < _width; ++x)
	{
		for (int z = 0; z < _depth; ++z)
		{
			int index = x + 2 * _width + z * _width * _height;
			_grid[index] = 1;
		}
	}
	*/
}

void World::_buildRenderBuffer()
{
	int i = glGetError();
	cout << "STARTING RENDER BUFFER BUILD (" << i << ")" << endl;

	// defines the 8 vertices of the cube, starting at lower left, going CCW, and front face to back face
	static float cubeVertexData[] = {-0.5f, -0.5f,  0.5f,
									  0.5f, -0.5f,  0.5f,
									  0.5f,  0.5f,  0.5f,
									 -0.5f,  0.5f,  0.5f,
									 -0.5f, -0.5f, -0.5f,
									  0.5f, -0.5f, -0.5f,
									  0.5f,  0.5f, -0.5f,
									  -0.5f,  0.5f, -0.5f};
	
	// defines the index data to draw the 12 triangles of the cube, CCW winding
	static unsigned short cubeIndexData[] = {1,5,6,1,6,2,	// +X face
											 4,0,3,4,3,7,	// -X face
											 3,2,6,3,6,7,	// +Y face
											 4,5,1,4,1,0,	// -Y face
											 0,1,2,0,2,3,	// +Z face
											 5,4,7,5,7,6};	// -Z face

	// each "quad" of each face has the same UV layout
	static float cubeUVData[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

	try
	{
		// initialize vector to the size of 1 y-slice of the whole grid, just to avoid constant realloc's
		int initialSize = _width * _depth;
		vector<FaceData> _visibleVoxels(initialSize);
		int faceCount = 0, n;

		// ======================================== TERRAIN ========================================

		// first, determine which solid voxels have exposed faces to empty/water voxels, and compile a list with
		// the integer coords of the voxel and the face flags (bits 0 through 5)
		for (int x = 0; x < _width; ++x)
		{
			for (int y = 0; y < _height; ++y)
			{
				for (int z = 0; z < _depth; ++z)
				{
					int index = x + y * _width + z * _width * _height;
					// only check soil voxels
					if (_grid[index] == 1)
					{
						// find all visible faces and save in faceFlags
						FaceData fd;
						// initially, set as 0 to indicate no visible faces
						fd.faceFlags = 0;
						fd.x = x; fd.y = y; fd.z = z;
						n = _computeVisibleFaces(x, y, z, &fd, true);

						// if faceFlags is still 0, entire voxel is hidden, otherwise save and count faces
						if (fd.faceFlags != 0)
						{
							_visibleVoxels.push_back(fd);
							faceCount += n;
						}
					}
				}
			}
		}

		// next, allocate room for all vertex data that will comprise the visible faces.
		// each vertex has 5 floats: x, y, z, u, and v.  the vertex data will be first, then
		// the uv data.  terrain.buffer.offset stores the offset to the uv data.
		int vertDataSize = 5;
		Segment terrain;
		terrain.buffer.data = new float[faceCount * 2 * 3 * vertDataSize];
		terrain.buffer.len = faceCount * 2 * 3 * vertDataSize * sizeof(float);
		terrain.buffer.offset = faceCount * 2 * 3 * 3 * sizeof(float);
		terrain.buffer.elementCount = faceCount * 2 * 3;
		float* ptr = terrain.buffer.data;
		float* ptr2 = terrain.buffer.data + faceCount * 2 * 3 * 3;
		
		// then, iterate the vector of voxels with exposed faces and build a buffer of triangle data and uv data.
		// this data is just sequential GL_TRIANGLES; no indexed drawing is used.
		for (auto i = _visibleVoxels.begin(); i != _visibleVoxels.end(); ++i)
		{
			// since the cube is 1x1x1 and centered at the origin, bias the data by 0.5 plus the voxel coord
			float dx = static_cast<float>(i->x) + 0.5f;
			float dy = static_cast<float>(i->y) + 0.5f;
			float dz = static_cast<float>(i->z) + 0.5f;

			// loop through each face, 0 to 5
			for (int face = 0; face < 6; ++face)
			{
				// if that face's bit is set, it's visible
				if (i->faceFlags & (1 << face))
				{
					// loop through the 6 verts of the 2 triangles of the face
					for (int v = 0; v < 6; ++v)
					{
						// offset the object-space indexed cube vertex by the current voxel position
						//memcpy(ptr, &cubeVertexData[cubeIndexData[face * 6 + v] * 3], sizeof(float) * 3); ptr += 3;
						//memcpy(ptr2, &cubeUVData[v * 2], sizeof(float) * 2); ptr2 += 2;
						
						*ptr = cubeVertexData[cubeIndexData[face * 6 + v] * 3 + 0] + dx; ptr++;
						*ptr = cubeVertexData[cubeIndexData[face * 6 + v] * 3 + 1] + dy; ptr++;
						*ptr = cubeVertexData[cubeIndexData[face * 6 + v] * 3 + 2] + dz; ptr++;
						*ptr2 = cubeUVData[v * 2 + 0]; ptr2++;
						*ptr2 = cubeUVData[v * 2 + 1]; ptr2++;
						
					}
				}
			}
		}

		// finally, create the VAO, VBO, etc and upload data to the card, and add this segment to the vector
		glGenVertexArrays(1, &terrain.vao);
		glBindVertexArray(terrain.vao);

		glGenBuffers(1, &terrain.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, terrain.vbo);
		glBufferData(GL_ARRAY_BUFFER, terrain.buffer.len, terrain.buffer.data, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		// stream 0 is position data, 3 floats
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
		// stream 3 is uv data, 2 floats
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)terrain.buffer.offset);

		// done defining the VAO, now add to the vector
		glBindVertexArray(0);
		_segments.push_back(terrain);

		// ======================================== WATER ========================================

		_visibleVoxels.clear();
		faceCount = 0;
		// repeat the same finding process with the water voxels, but only find the faces that are adjacent to empty voxels
		for (int x = 0; x < _width; ++x)
		{
			for (int y = 0; y < _height; ++y)
			{
				for (int z = 0; z < _depth; ++z)
				{
					int index = x + y * _width + z * _width * _height;
					// only check water voxels
					if (_grid[index] == 2)
					{
						// find all visible faces and save in faceFlags
						FaceData fd;
						// initially, set as 0 to indicate no visible faces
						fd.faceFlags = 0;
						fd.x = x; fd.y = y; fd.z = z;
						n = _computeVisibleFaces(x, y, z, &fd, false);

						// if faceFlags is still 0, entire voxel is hidden, otherwise save and count faces
						if (fd.faceFlags != 0)
						{
							_visibleVoxels.push_back(fd);
							faceCount += n;
						}
					}
				}
			}
		}

		// same as above, allocate the buffer and fill it up
		//int vertDataSize = 5;
		Segment water;
		water.buffer.data = new float[faceCount * 2 * 3 * vertDataSize];
		water.buffer.len = faceCount * 2 * 3 * vertDataSize * sizeof(float);
		water.buffer.offset = faceCount * 2 * 3 * 3 * sizeof(float);
		water.buffer.elementCount = faceCount * 2 * 3;
		ptr = water.buffer.data;
		ptr2 = water.buffer.data + faceCount * 2 * 3 * 3;
		
		// then, iterate the vector of voxels with exposed faces and build a buffer of triangle data and uv data.
		// this data is just sequential GL_TRIANGLES; no indexed drawing is used.
		for (auto i = _visibleVoxels.begin(); i != _visibleVoxels.end(); ++i)
		{
			// since the cube is 1x1x1 and centered at the origin, bias the data by 0.5 plus the voxel coord
			float dx = static_cast<float>(i->x) + 0.5f;
			float dy = static_cast<float>(i->y) + 0.5f;
			float dz = static_cast<float>(i->z) + 0.5f;

			// loop through each face, 0 to 5
			for (int face = 0; face < 6; ++face)
			{
				// if that face's bit is set, it's visible
				if (i->faceFlags & (1 << face))
				{
					// loop through the 6 verts of the 2 triangles of the face
					for (int v = 0; v < 6; ++v)
					{
						// offset the object-space indexed cube vertex by the current voxel position
						//memcpy(ptr, &cubeVertexData[cubeIndexData[face * 6 + v] * 3], sizeof(float) * 3); ptr += 3;
						//memcpy(ptr2, &cubeUVData[v * 2], sizeof(float) * 2); ptr2 += 2;
						
						*ptr = cubeVertexData[cubeIndexData[face * 6 + v] * 3 + 0] + dx; ptr++;
						*ptr = cubeVertexData[cubeIndexData[face * 6 + v] * 3 + 1] + dy; ptr++;
						*ptr = cubeVertexData[cubeIndexData[face * 6 + v] * 3 + 2] + dz; ptr++;
						*ptr2 = cubeUVData[v * 2 + 0]; ptr2++;
						*ptr2 = cubeUVData[v * 2 + 1]; ptr2++;
						
					}
				}
			}
		}

		// finally, create the VAO, VBO, etc and upload data to the card, and add this segment to the vector
		glGenVertexArrays(1, &water.vao);
		glBindVertexArray(water.vao);

		glGenBuffers(1, &water.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, water.vbo);
		glBufferData(GL_ARRAY_BUFFER, water.buffer.len, water.buffer.data, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		// stream 0 is position data, 3 floats
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
		// stream 3 is uv data, 2 floats
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)water.buffer.offset);

		// done defining the VAO, now add to the vector
		glBindVertexArray(0);
		_segments.push_back(water);

		int i = glGetError();
		cout << "DONE WITH RENDER BUFFER BUILD (" << i << ")" << endl;
	}
	catch (exception e)
	{
		cout << "error building render buffer (" << e.what() << ")" << endl;
	}
}

// checks which of the 6 faces of a voxel are adjacent to an empty voxel or, optionally, water, set by bool checkWater
// result is saved in the provided FaceData pointer
int World::_computeVisibleFaces(int x, int y, int z, FaceData* fd, bool checkWater)
{
	// the various offsets used to check adjacent voxels
	static int dx[] = {1,-1,0,0,0,0};
	static int dy[] = {0,0,1,-1,0,0};
	static int dz[] = {0,0,0,0,1,-1};
	int faceCount = 0;

	if (fd != NULL)
	{
		// loop through faces 0 to 5, +x, -x, +y, -y, +z, and -z.
		for (int face = 0; face < 6; ++face)
		{
			// compute adjacent voxel coord
			int tx = x + dx[face];
			int ty = y + dy[face];
			int tz = z + dz[face];

			// check if the current adjacent voxel coord is out of bounds, which will happen with edge voxels
			if (!((tx < 0) || (tx >= _width) || (ty < 0) || (ty >= _height) || (tz < 0) || (tz >= _depth)))
			{
				int index = tx + ty * _width + tz * _width * _height;
				// if it's empty (0) or water (2), then this face is visible
				if (_grid[index] == 0 || (_grid[index] == 2 && checkWater))
				{
					fd->faceFlags |= (1 << face);
					faceCount++;
				}
			}
		}
	}

	return faceCount;
}

void World::render(Camera& camera, ShaderProgram& program)
{
	static MatrixStack t;

	t.makeIdentity();

	/*
	for (auto i = _segments.begin(); i != _segments.end(); ++i)
	{
		glBindVertexArray(i->vao);
		glDrawArrays(GL_TRIANGLES, 0, i->buffer.elementCount);
	}*/

	// enabling texturing, and set the color to white
	program.setUniform1i("useTexturing", 1);
	_voxelDefs[0].texture->bind(0);
	glVertexAttrib3f(1, 1.0f, 1.0f, 1.0f);

	// scale the texture samples by their y height
	program.setUniform1i("useHeightFactor", 1);
	
	// setup matrices
	camera.setWorldMatrix(t.top());
	camera.updateShader();

	// draw!
	glBindVertexArray(_segments[0].vao);
	glDrawArrays(GL_TRIANGLES, 0, _segments[0].buffer.elementCount);

	// blend the water segment against the terrain segment
	glEnable(GL_BLEND);
	//glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	_voxelDefs[1].texture->bind(0);

	// water doesn't use the height scaling
	program.setUniform1i("useHeightFactor", 0);

	glBindVertexArray(_segments[1].vao);
	glDrawArrays(GL_TRIANGLES, 0, _segments[1].buffer.elementCount);

	glDisable(GL_BLEND);
	

	/*
	// brute force!
	for (int x = 0; x < _width; ++x)
	{
		for (int y = 0; y < _height; ++y)
		{
			for (int z = 0; z < _depth; ++z)
			{
				int index = x + y * _width + z * _width * _height;
				if (_grid[index])
				{
					t.push();
						t.translate(static_cast<float>(x - _width / 2), static_cast<float>(y - _height / 2), static_cast<float>(z - _depth / 2));
						t.scale(0.5f, 0.5f, 0.5f);
						camera.setWorldMatrix(t.top());
						camera.updateShader();
						_object.render();
					t.pop();
				}
			}
		}
	}
	*/
}