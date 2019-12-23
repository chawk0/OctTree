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
	for (auto i = _voxelDefs.begin(); i != _voxelDefs.end(); ++i)
		if (i->second.texture != NULL)
			delete i->second.texture;
	
	// delete the VAO, VBO, and buffers for each segment
	for (auto i = _segments.begin(); i != _segments.end(); ++i)
	{
		glDeleteVertexArrays(1, &i->second.vao);
		delete [] i->second.buffer.data;
		glDeleteBuffers(1, &i->second.vbo);
		delete [] i->second.visibleThroughTypes;
	}
	delete [] _segmentRenderList;
};

void World::create(const char* file)
{
	try
	{
		_loadVoxelDefs();
		_loadSegments(file);
		_buildRenderBuffer();
	}
	catch (std::bad_alloc& info)
	{
		cout << "failed to allocate world data (" << info.what() << ")" << endl;
	}
	catch (exception e)
	{
		cout << "unknown error in world create (" << e.what() << ")" << endl;
	}				
}

void World::buildOctree()
{
	octree.build(_grid, _width, 10);
}

void World::temp()
{
	_width = _height = _depth = 512;
	_grid = new unsigned int[_width * _height * _depth];
	memset(_grid, 0, sizeof(unsigned int) * _width * _height * _depth);

	_loadVoxelDefs();

	unsigned int waterHeight = static_cast<unsigned int>(512.0f * 0.35f);
	unsigned int id = _voxelDefs["water"].id;
	//unsigned int waterHeight = static_cast<unsigned int>(3);

	// loop over the XZ plane and fill columns of voxels
	for (unsigned int z = 0; z < _depth; ++z)
	{
		for (unsigned int y = 0; y < _height; ++y)
		{
			for (unsigned int x = 0; x < _width; ++x)			
			{
				unsigned long long index = x + y * _width + z * _width * _height;
				if (y <= waterHeight)
				//if ((y >= 0) && (y < waterHeight) || (y >= (_height - waterHeight)) && (y < _height))
				//if ((y >= 1) && (y < 5) && (x >= 1) && (x < 5) && (z >= 1) && (z < 5)) 
					_grid[index] = id;
				else
					_grid[index] = 0;
			}
		}
	}

	Segment newSegment;
	newSegment.voxelName = "water";
	newSegment.visibleThroughTypesSize = 0;
	_segments["water"] = newSegment;
	saveSegment("water");
}

void World::saveSegment(const char* name)//, const char* file)
{
	string name2 = string(name);
	string defFile = "./res/segments/" + name2 + ".txt";
	string dataFile = "./res/segments/" + name2 + ".dat";

	// writes the segment's data to a segment def .txt file, and creates a .dat file with the voxel data
	auto s = _segments.find(name2);
	if (s != _segments.end())
	{
		ofstream fileOut;
		// write the segment def .txt file
		fileOut.open(defFile.c_str(), ios::out);

		fileOut << "segment" << endl;
		fileOut << "name " << s->first << endl;
			
		fileOut << "madeof " << s->second.voxelName << endl;

		fileOut << "visiblethrough " << s->second.visibleThroughTypesSize << " ";
		for (unsigned int i = 0; i < s->second.visibleThroughTypesSize; ++i)
			fileOut << s->second.visibleThroughTypes[i] << " ";
		fileOut << endl;
		if (s->second.useBlend)
			fileOut << "blend" << endl;

		fileOut << "data file " << dataFile << endl;
		fileOut << "end" << endl;

		fileOut.close();

		// write the segment data .dat file
		unsigned int id = _voxelDefs[s->second.voxelName].id;
		unsigned int one = 1, zero = 0;
		
		fileOut.open(dataFile.c_str(), ios::out | ios::binary);

		for (unsigned long long j = 0; j < (_width * _height * _depth); ++j)
		{
			if (_grid[j] == id)
				fileOut.write(reinterpret_cast<const char*>(&one), 4);
			else
				fileOut.write(reinterpret_cast<const char*>(&zero), 4);
		}
		fileOut.close();
	}
}

void World::create(int width, int height, int depth)
{
	/*
	try
	{
		if (_grid != NULL)
			delete [] _grid;

		_grid = new unsigned int[width * height * depth];
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
	*/
}

void World::_loadVoxelDefs()
{
	ifstream fileIn;
	string token, key;
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
			token = "";
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
					fileIn >> key;
					cout << "\tvoxel name: " << key << endl;
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
					_voxelDefs[key] = newVoxel;
					// ensure each new voxel def has a NULL texture pointer by default. makes cleanup easier
					newVoxel.texture = NULL;
				}
				else
					cout << "unknown token in voxel def: " << token << endl;
			}
		}

		fileIn.close();
	}
	catch (exception e)
	{
		cout << "exception loading voxel defs (" << e.what() << ")" << endl;
	}
}

void World::_loadSegments(const char* file)
{
	ifstream fileIn;
	string token, key;
	char buffer[261];
	bool inSegment = false;
	Segment newSegment;

	// load the world parameters and segments from file. each segment represents a grouping of voxels that
	// are treated as one unit when rendering, e.g. soil voxels form the soil segment, water forms another, etc.
	// segments overwrite or "paint" into the world grid in order, either by loading data from a file or generating
	// Perlin noise.
	cout << "loading world file " << file << "..." << endl;

	fileIn.open(file, ios::in);

	while (!fileIn.eof() && !fileIn.fail())
	{
		token = "";
		fileIn >> token;

		if (token.length() > 0)
		{
			// skip comments
			if (token[0] == '#')
				fileIn.getline(buffer, 260);
			else if (token == "name")
			{
				fileIn >> token;

				// either name the world or save the key for the segment map
				if (!inSegment)
				{
					_name = token;
					cout << "name " << token << endl;
				}
				else
				{
					key = token;
					cout << "new segment " << key << endl;
				}
			}
			else if (token == "size")
			{
				unsigned int size;
				fileIn >> size;
				_width = _height = _depth = size;
				cout << "size " << size << "x" << size << "x" << size << endl;
			}
			else if (token == "segment")
			{
				inSegment = true;
				// make sure these are initialized each time
				newSegment.faceCount = 0;
				newSegment.useBlend = false;
			}
			else if (token == "end")
			{
				inSegment = false;
				newSegment.faceCount = 0;
				// add segment to the map
				_segments[key] = newSegment;
				// somewhat of a hack to enable mapping in the other direction. a given voxel id is used
				// as a key to map to the segment that is composed of that voxel id.
				_segmentIDMap[_voxelDefs[newSegment.voxelName].id] = key;
			}
			else if (token == "madeof")
			{
				// voxel name that this segment is comprised of
				fileIn >> token;
				cout << "\tmadeof " << token << endl;
				newSegment.voxelName = token;
				cout << endl;
			}
			else if (token == "visiblethrough")
			{
				// list of voxel ids that this segment is visible through, like water etc.
				// note, all segments are visible through the null block, id = 0
				int count, id;
				fileIn >> count;
				newSegment.visibleThroughTypes = new unsigned int[count];
				newSegment.visibleThroughTypesSize = count;
				cout << "\t" << count << " visiblethrough types ";
				for (int i = 0; i < count; ++i)
				{
					fileIn >> id;
					//newSegment.visibleThroughTypes.push_back(id);
					newSegment.visibleThroughTypes[i] = id;
					cout << id << " ";
				}
				cout << endl;
			}
			else if (token == "blend")
				newSegment.useBlend = true;
			else if (token == "data")
			{
				// describes how the segment data is acquired: either loaded from a file or procedurally generated
				fileIn >> token;

				if (token == "file")
				{
					newSegment.dataSource = Segment::DataSourceType::File;

					fileIn.getline(buffer, 260);
					token = string(buffer);

					// trim leading and trailing whitespace
					auto first = token.find_first_not_of(" \t");
					auto len = token.find_last_not_of(" \t") - first + 1;
					token = token.substr(first, len);

					newSegment.dataFile = token;
					cout << "\tdata file " << token << endl;
				}
				else if (token == "perlin")
				{
					newSegment.dataSource = Segment::DataSourceType::Perlin;

					fileIn >> newSegment.perlinFrequency >> newSegment.perlinPersistence;
					cout << "\tperlin noise " << newSegment.perlinFrequency << " " << newSegment.perlinPersistence << endl;
				}
			}
			else
			{
				cout << "unknown token " << token << endl;
			}
		}
	}

	fileIn.close();

	// allocate space for the world grid
	_grid = new unsigned int[_width * _height * _depth];
	memset(_grid, 0, sizeof(unsigned int) * _width * _height * _depth);
	unsigned int* ptr;
	unsigned int id;

	// now iterate over each new segment, acquire its data and accumulate into the world grid.
	for (auto i = _segments.begin(); i != _segments.end(); ++i)
	{
		// set start of write pointer to the beginning of the grid
		ptr = _grid;
		// save the voxel id for this segment to avoid constant map lookups
		id = _voxelDefs[i->second.voxelName].id;

		// load from disk
		if (i->second.dataSource == Segment::DataSourceType::File)
		{
			cout << "loading segment data from " << i->second.dataFile << "..." << endl;
			
			ifstream fileIn;
			// 1024 dwords is 4KB of data from disk, ideally an optimal chunk size for sequential reading of large files?
			unsigned int chunk[1024];
			fileIn.open(i->second.dataFile, ios::in | ios::binary);
			fileIn.read(reinterpret_cast<char*>(chunk), 4096);

			while (!fileIn.eof() && !fileIn.fail())
			{
				// union the data of this chunk with the grid data.  voxels in the chunk that are zero leave the existing
				// grid data unchanged.
				for (int j = 0; j < 1024; ++j)
				{
					if (chunk[j] == 1)
						*ptr = id;
					ptr++;
				}

				// get the next 4KB chunk
				fileIn.read(reinterpret_cast<char*>(chunk), 4096);
			}

			// the while loop ends once we're at the end of the file, so fill the remaining voxels.
			// it also could fail for some reason, but hopefully not...
			// gcount() gives the remaining byte count, so divide by 4 to iterate by dwords
			int remainder = static_cast<int>(fileIn.gcount()) / 4;
			for (int j = 0; j < remainder; j++)
			{
				if (chunk[j] == 1)
					*ptr = id;
				ptr++;
			}

			fileIn.close();
			/*
			for (unsigned int z = 0; z < _depth; ++z)
			{
				for (unsigned int y = 0; y < 60; ++y)
				{
					for (unsigned int x = 0; x < _width; ++x)
					{
						unsigned long long index = x + y * _width + z * _width * _height;
						_grid[index] = id;
					}
				}
			}
			*/
			cout << "done" << endl;
		}
		else if (i->second.dataSource == Segment::DataSourceType::Perlin)
		{
			cout << "generating Perlin noise..." << endl;
			// generate random perlin noise in the XZ direction, with the noise value becoming terrain height.
			// the column of voxels under the top voxel is filled in, so we get a downward fill
			for (unsigned int x = 0; x < _width; ++x)
			{
				for (unsigned int z = 0; z < _depth; ++z)
				{
					// the Perlin noise functions return doubles between [-1.0, 1.0] but the distribution varies with persistence.
					// bias the value to [0.0, 1.0] then scale by grid height
					unsigned int maxY = static_cast<unsigned int>((Perlin::PerlinNoise2D(static_cast<double>(x), static_cast<double>(z), i->second.perlinFrequency, i->second.perlinPersistence) / 2.0 + 0.5) * static_cast<double>(_height));
						
					// fill the column of voxels up to maxY
					for (unsigned int y = 0; y < maxY; ++y)
					{
						unsigned long long index = x + y * _width + z * _width * _height;
						_grid[index] = id;
					}			
				}
			}
			cout << "done" << endl;
		}
	}
}

void World::_generateLandscape()
{
	/*
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
	*/
	
	/*
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
	*/
	
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
		Segment* segPtr[100];
		// a hack to allow quick STL-free mapping from voxel id to Segment*. this is useful to
		// quickly access the visibleThroughTypes array for the given Segment.
		for (auto vd = _voxelDefs.begin(); vd != _voxelDefs.end(); ++vd)
		{
			auto s = _segmentIDMap.find(vd->second.id);
			if (s != _segmentIDMap.end())
				segPtr[vd->second.id] = &_segments[s->second];
		}
		
		unsigned int v, n;
		// first, determine which solid voxels have exposed faces and compile a list with
		// the integer coords of the voxel and the face flags (bits 0 through 5).  the voxel id
		// of the current voxel is mapped to the segment made of that same id, which then supplies
		// the computeVisibleFaces function with the correct visiblethrough list
		for (unsigned int z = 0; z < _depth; ++z)
		{
			for (unsigned int y = 0; y < _height; ++y)
			{
				for (unsigned int x = 0; x < _width; ++x)
				{
					unsigned long long index = x + y * _width + z * _width * _height;
					v = _grid[index];

					// only check non-empty voxels
					if (v != 0)
					{
						// find all visible faces and save in faceFlags
						FaceData fd;
						// initially, set as 0 to indicate no visible faces
						fd.faceFlags = 0;
						fd.x = x; fd.y = y; fd.z = z;
						// sort-of hack that allows me to map a voxel id back to the segment it was generated from
						//s = &(_segments[_segmentIDMap[v]]);
						n = _computeVisibleFaces(x, y, z, &fd, segPtr[v]->visibleThroughTypes, segPtr[v]->visibleThroughTypesSize);

						// if faceFlags isn't 0, it has n visible faces, so save it
						if (fd.faceFlags != 0)
						{
							segPtr[v]->visibleVoxels.push_back(fd);
							segPtr[v]->faceCount += n;	
						}						
					}
				}
			}
		}
		
		// next, allocate room for all vertex data that will comprise the visible faces.
		// each vertex has 5 floats: x, y, z, u, and v.  the vertex data will be first, then
		// the uv data.  segment.buffer.offset stores the offset to the uv data.
		for (auto s = _segments.begin(); s != _segments.end(); ++s)
		{
			int vertDataSize = 5;
			s->second.buffer.data = new float[s->second.faceCount * 2 * 3 * vertDataSize];
			s->second.buffer.size = s->second.faceCount * 2 * 3 * vertDataSize * sizeof(float);
			s->second.buffer.offset = s->second.faceCount * 2 * 3 * 3 * sizeof(float);
			s->second.buffer.elementCount = s->second.faceCount * 2 * 3;
			float* ptr = s->second.buffer.data;
			float* ptr2 = s->second.buffer.data + s->second.faceCount * 2 * 3 * 3;

			cout << "building segment " << s->first << endl;
			cout << "\tfacecount " << s->second.faceCount << endl;
			cout << "\tbuffer size " << s->second.buffer.size << endl;
		
			// then, iterate the vector of voxels with exposed faces and build a buffer of triangle data and uv data.
			// this data is just sequential GL_TRIANGLES; no indexed drawing is used.
			for (auto i = s->second.visibleVoxels.begin(); i != s->second.visibleVoxels.end(); ++i)
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
			glGenVertexArrays(1, &s->second.vao);
			glBindVertexArray(s->second.vao);

			glGenBuffers(1, &s->second.vbo);
			glBindBuffer(GL_ARRAY_BUFFER, s->second.vbo);
			glBufferData(GL_ARRAY_BUFFER, s->second.buffer.size, s->second.buffer.data, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			// stream 0 is position data, 3 floats
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(3);
			// stream 3 is uv data, 2 floats
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)s->second.buffer.offset);

			// done defining the VAO
			glBindVertexArray(0);
			cout << "done" << endl;
		}

		// allocate the segment render list and fill it with the segments.
		// move any segments with blend enabled to the end
		_segmentRenderList = new Segment*[_segments.size()];
		_segmentRenderListSize = static_cast<unsigned int>(_segments.size());
		Segment** ptr = _segmentRenderList;
		// move non-blended segments to the front
		for (auto i = _segments.begin(); i != _segments.end(); ++i)
			if (!i->second.useBlend)
			{
				*ptr = &(i->second);
				ptr++;
			}
		// and fill in the blended segments at the end
		for (auto i = _segments.begin(); i != _segments.end(); ++i)
			if (i->second.useBlend)
			{
				*ptr = &(i->second);
				ptr++;
			}


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
unsigned int World::_computeVisibleFaces(unsigned int x, unsigned int y, unsigned int z, FaceData* fd, unsigned int* vtt, unsigned int vttSize)
{
	// the various offsets used to check adjacent voxels
	static int dx[] = {1,-1,0,0,0,0};
	static int dy[] = {0,0,1,-1,0,0};
	static int dz[] = {0,0,0,0,1,-1};
	int faceCount = 0;
	bool isVisible;

	if (fd != NULL)
	{
		// loop through faces 0 to 5, +x, -x, +y, -y, +z, and -z.
		for (int face = 0; face < 6; ++face)
		{
			// compute adjacent voxel coord
			int tx = static_cast<int>(x) + dx[face];
			int ty = static_cast<int>(y) + dy[face];
			int tz = static_cast<int>(z) + dz[face];

			isVisible = false;

			// check if the current adjacent voxel coord is out of bounds, which will happen with edge voxels
			if (!((tx < 0) || (tx >= static_cast<int>(_width)) || (ty < 0) || (ty >= static_cast<int>(_height)) || (tz < 0) || (tz >= static_cast<int>(_depth))))
			{
				unsigned long long index = static_cast<unsigned int>(tx) + static_cast<unsigned int>(ty) * _width + static_cast<unsigned int>(tz) * _width * _height;

				// if the voxel is empty, then the face is visible
				if (_grid[index] == 0)
					isVisible = true;
				// also ensure there's a list to compare to at all
				else if (vttSize > 0)
				{
					// loop through the visiblethrough types and check if the adjacent voxel is any of these types
					for (unsigned int i = 0; i < vttSize; ++i)
					{
						if (_grid[index] == vtt[i])
						{
							isVisible = true;
							break;
						}
					}
				}

				if (isVisible)
				{
					fd->faceFlags |= (1 << face);
					faceCount++;
				}
			}
		}
	}

	return faceCount;
}

void World::render(Shader* shader, bool hideTerrain)
{
	if (!hideTerrain)
	{
		// render all the segments
		shader->setUniform1i("useTexturing", 1);
		glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);

		for (unsigned int i = 0; i < _segmentRenderListSize; ++i)
		{
			Segment* s = _segmentRenderList[i];		
			_voxelDefs[s->voxelName].texture->bind(0);

			// segments that use blend should be last in the list
			if (s->useBlend)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glBindVertexArray(s->vao);
				glDrawArrays(GL_TRIANGLES, 0, s->buffer.elementCount);

				glDisable(GL_BLEND);
			}
			// hack to make soil nicer looking
			else if (s->voxelName == "soil")
			{
				shader->setUniform1i("useHeightFactor", 1);
				shader->setUniform1f("worldHeight", static_cast<float>(_height));
				glBindVertexArray(s->vao);
				glDrawArrays(GL_TRIANGLES, 0, s->buffer.elementCount);
				shader->setUniform1i("useHeightFactor", 0);
			}
			// regular draw
			else
			{
				glBindVertexArray(s->vao);
				glDrawArrays(GL_TRIANGLES, 0, s->buffer.elementCount);
			}		
		}
	}

	// render the octree wireframe
	octree.render(shader);
}