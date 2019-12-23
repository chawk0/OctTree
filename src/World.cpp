#include "World.h"


World::World():
	_grid(NULL),
	_width(0), _height(0), _depth(0),
	_voxelDefs(NULL)	
{
	//
}

World::~World()
{
	destroy();
};

void World::create(const char* file)
{
	try
	{
		cout << "loading segments..." << endl;
		_loadSegments(file);
		cout << "computing visible faces..." << endl;
		_computeVisibleFaces();
		cout << "building render buffer..." << endl;
		_buildRenderBuffer();

		/*
		cout << endl << endl;
		for (uint i = 0; i < _segments.size(); ++i)
		{
			cout << "_segments[" << i << "]" << endl;
			cout << "\tname: " << _segments[i].name << endl;
			cout << "\tvoxelID: " << _segments[i].voxelID << endl;
			cout << "\tvisibleThroughIDs.size(): " << _segments[i].visibleThroughIDs.size() << endl;
			if (_segments[i].visibleThroughIDs.size() > 0)
			{
				cout << "\t\t";
				for (uint j = 0; j < _segments[i].visibleThroughIDs.size(); ++j)
					cout << _segments[i].visibleThroughIDs[j] << " ";
				cout << endl;
			}
			cout << "\tuseBlend: " << _segments[i].useBlend << endl;
			cout << "\tdataSourceType: " << _segments[i].dataSourceType << endl;
			cout << "\tdataSourceFile: " << _segments[i].dataSourceFile << endl;
			cout << "\tPerlin freq: " << _segments[i].perlinFrequency << ", pers: " << _segments[i].perlinPersistence << ", amp: " << _segments[i].amplitude << endl;
			cout << "\tvisibleVoxels.size(): " << _segments[i].visibleVoxels.size() << endl;
			cout << "\tvisibleFaceCount: " << _segments[i].visibleFaceCount << endl;
			cout << "\tvao: " << _segments[i].vao << endl;
			cout << "\tvertexBuffer: " << endl;
			cout << "\t\tvbo: " << _segments[i].vertexBuffer.vbo << endl;
			cout << "\t\ttarget: " << _segments[i].vertexBuffer.target << endl;
			cout << "\t\tsize: " << _segments[i].vertexBuffer.size << endl;
			cout << "\t\tdata: " << _segments[i].vertexBuffer.data << endl;
			cout << "\t\tusage: " << _segments[i].vertexBuffer.usage << endl;
			cout << "\tvertexAttribs.size(): " << _segments[i].vertexAttribs.size() << endl;
			if (_segments[i].vertexAttribs.size() > 0)
			{
				for (uint j = 0; j < _segments[i].vertexAttribs.size(); ++j)
				{
					cout << "\t[" << j << "]:" << endl;
					cout << "\t\tname: " << _segments[i].vertexAttribs[j].name << endl;
					cout << "\t\tindex: " << _segments[i].vertexAttribs[j].index << endl;
					cout << "\t\tcomponentCount: " << _segments[i].vertexAttribs[j].componentCount << endl;
					cout << "\t\tdataType: " << _segments[i].vertexAttribs[j].dataType << endl;
					cout << "\t\tstride: " << _segments[i].vertexAttribs[j].stride << endl;
					cout << "\t\toffset: " << _segments[i].vertexAttribs[j].offset << endl;
				}
			}
			cout << "\telementCount: " << _segments[i].elementCount << endl;
			cout << "\trenderMode: " << _segments[i].renderMode << endl;
		}
		cout << endl << endl;
		*/
	}
	catch (std::bad_alloc& info)
	{
		cout << "ERROR: World::create: failed to allocate world grid (" << info.what() << ")" << endl;
	}
	catch (exception e)
	{
		cout << "ERROR: World::create: unknown error (" << e.what() << ")" << endl;
	}				
}

void World::destroy()
{
	cout << "destroying world..." << endl;

	// free the world grid
	if (_grid != NULL)
	{
		delete [] _grid;
		_grid = NULL;
	}
	_width = _height = _depth = 0;
	
	// delete the VAO, VBO, and buffers for each segment
	for (uint i = 0; i < _segments.size(); ++i)
	{
		// delete VAO
		glDeleteVertexArrays(1, &_segmentRenderData[i].vao);

		// delete VBO and system memory data (if present)
		for (uint j = 0; j < _segmentRenderData[i].vertexBuffers.size(); j++)
		{
			glDeleteBuffers(1, &_segmentRenderData[i].vertexBuffers[j].vbo);
			_segmentRenderData[i].vertexBuffers[j].vbo = 0;
			if (_segmentRenderData[i].vertexBuffers[j].data != NULL)
			{
				delete [] _segmentRenderData[i].vertexBuffers[j].data;
				_segmentRenderData[i].vertexBuffers[j].data = NULL;
			}
		}

		// delete IBO and system memory data (if present)
		for (uint j = 0; j < _segmentRenderData[i].indexBuffers.size(); j++)
		{
			glDeleteBuffers(1, &_segmentRenderData[i].indexBuffers[j].ibo);
			_segmentRenderData[i].indexBuffers[j].ibo = 0;
			if (_segmentRenderData[i].indexBuffers[j].data != NULL)
			{
				delete [] _segmentRenderData[i].indexBuffers[j].data;
				_segmentRenderData[i].indexBuffers[j].data = NULL;
			}
		}

		_segmentRenderData[i].vertexAttribs.free();
		_segmentRenderData[i].elementCount = 0;
	}

	// free the arrays
	_segments.free();
	_segmentRenderData.free();
	_segmentRenderList.free();
	_segmentMap.free();		
}

uint* World::getGrid()
{
	return _grid;
}

uint World::getGridSize()
{
	return _width;
}

Array<Segment>* World::getSegments()
{
	return &_segments;
}

Array<uint>* World::getSegmentMap()
{
	return &_segmentMap;
}

Array<uint>* World::getSegmentRenderList()
{
	return &_segmentRenderList;
}

void World::temp()
{
	/*
	_width = _height = _depth = 512;
	_grid = new uint[_width * _height * _depth];
	memset(_grid, 0, sizeof(uint) * _width * _height * _depth);

	_loadVoxelDefs();

	uint waterHeight = static_cast<uint>(512.0f * 0.35f);
	uint id = _voxelDefs["water"].id;
	//uint waterHeight = static_cast<uint>(3);

	// loop over the XZ plane and fill columns of voxels
	for (uint z = 0; z < _depth; ++z)
	{
		for (uint y = 0; y < _height; ++y)
		{
			for (uint x = 0; x < _width; ++x)			
			{
				uint64 index = x + y * _width + z * _width * _height;
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
	*/
}

void World::saveSegment(const char* name)
{
	string name2 = string(name);
	string defFile = "./res/segments/" + name2 + ".txt";
	string dataFile = "./res/segments/" + name2 + ".dat";

	// writes the segment's data to a segment def .txt file, and creates a .dat file with the voxel data
	for (uint i = 0; i < _segments.size(); ++i)
	{
		if (_segments[i].name == string(name))
		{
			ofstream fileOut;
			// write the segment def .txt file
			fileOut.open(defFile.c_str(), ios::out);

			fileOut << "segment" << endl;
			fileOut << "name " << _segments[i].name << endl;
			
			fileOut << "madeof " << (*_voxelDefs)[_segments[i].voxelID].name << endl;

			fileOut << "visiblethrough " << _segments[i].visibleThroughIDs.size() << " ";
			for (uint j = 0; j < _segments[i].visibleThroughIDs.size(); ++j)
				fileOut << _segments[i].visibleThroughIDs[j] << " ";
			fileOut << endl;
			if (_segments[i].useBlend)
				fileOut << "blend" << endl;

			fileOut << "data file " << dataFile << endl;
			fileOut << "end" << endl;

			fileOut.close();

			// write the segment data .dat file
			uint one = 1, zero = 0;
		
			fileOut.open(dataFile.c_str(), ios::out | ios::binary);

			for (uint64 j = 0; j < (_width * _height * _depth); ++j)
			{
				if (_grid[j] == _segments[i].voxelID)
					fileOut.write(reinterpret_cast<const char*>(&one), 4);
				else
					fileOut.write(reinterpret_cast<const char*>(&zero), 4);
			}
			fileOut.close();
			break;
		}
	}
}

void World::setVoxelDefs(Array<Voxel>* voxelDefs)
{
	if (voxelDefs != NULL)
	{
		if (voxelDefs->size() > 0)
		{
			// save the voxeldefs ptr and allocate space for the map (still an array) that
			// maps a voxel id to the segment index that generates it (if any). defaults to 0xFFFFFFFF
			_voxelDefs = voxelDefs;
			_segmentMap.alloc(voxelDefs->size());
			_segmentMap.set(0xFFFFFFFF);
		}
		else
			cout << "ERROR: World::setVoxelDefs given empty voxel defs array" << endl;
	}
	else
		cout << "ERROR: World::setVoxelDefs given bad ptr" << endl;
}

void World::_loadSegments(const char* file)
{
	ifstream fileIn;
	string token;
	char buffer[261];
	bool inSegment = false;
	Segment newSegment;
	uint currentSegment = 0;
	unordered_map<string, uint> voxelIDMap;

	if (file == NULL)
		cout << "ERROR: World::_loadSegments: given bad file" << endl;
	else if (_voxelDefs == NULL)
		cout << "ERROR: World::_loadSegments: no voxel defs, can't load" << endl;
	else
	{
		// first, create a map between voxel names and their ids to simplify the loading code.
		// the first index in the voxel defs is id 0, which represents the null block, so skip it
		for (uint i = 1; i <_voxelDefs->size(); ++i)
			voxelIDMap[(*_voxelDefs)[i].name] = i;

		// load the world parameters and segments from file. each segment represents a grouping of voxels that
		// are treated as one unit when rendering, e.g. soil voxels form the soil segment, water forms another, etc.
		// segments overwrite or "paint" into the world grid in order, either by loading data from a file or generating
		// Perlin noise.
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

					// a "name" token is used for both world and segment names
					if (!inSegment)
					{
						// world name
						_name = token;
						cout << "name " << token << endl;
					}
					else
					{
						// segment name
						newSegment.name = token;
						cout << "new segment " << token << endl;
					}
				}
				else if (token == "size")
				{
					uint size;
					fileIn >> size;
					_width = _height = _depth = size;
					cout << "size " << size << "x" << size << "x" << size << endl;
				}
				else if (token == "segments")
				{
					uint count;
					fileIn >> count;
					_segments.alloc(count);
					_segmentRenderData.alloc(count);
				}
				else if (token == "segment")
				{
					inSegment = true;
					// make sure these are initialized each new segment
					newSegment.visibleFaceCount = 0;
					newSegment.useBlend = false;
				}
				else if (token == "end")
				{
					inSegment = false;
					_segments[currentSegment] = newSegment;
					_segmentMap[newSegment.voxelID] = currentSegment;
					currentSegment++;
				}
				else if (token == "madeof")
				{
					bool found = false;

					// voxel name that this segment is composed of
					fileIn >> token;
					// make sure the voxel def exists
					auto i = voxelIDMap.find(token);
					if (i != voxelIDMap.end())
					{
						newSegment.voxelID = i->second;
						cout << "\tmadeof " << token << "(" << i->second << ")" << endl;
						found = true;
					}

					if (!found)
						cout << "ERROR: World::_loadSegments: bad madeof voxel name:" << token << endl;
				}
				else if (token == "visiblethrough")
				{
					// list of voxel ids that this segment is visible through, like water etc.
					// note, all segments are visible through the null block, id = 0
					uint count;

					fileIn >> count;
					if (count > 0)
					{
						newSegment.visibleThroughIDs.alloc(count);
						cout << "\t" << count << " visiblethrough types: ";

						for (uint i = 0; i < count; ++i)
						{
							fileIn >> token;
							auto j = voxelIDMap.find(token);
							if (j != voxelIDMap.end())
							{
								newSegment.visibleThroughIDs[i] = j->second;
								cout << token << " (" << j->second << ")";
								if (i < (count - 1))
									cout << ", ";
							}
							else
								cout << "ERROR: World::_loadSegments: bad visiblethrough voxel name:" << token << endl;
						}
						
						cout << endl;
					}
				}
				else if (token == "blend")
					newSegment.useBlend = true;
				else if (token == "data")
				{
					// describes how the segment data is acquired: either loaded from a file or procedurally generated
					fileIn >> token;

					if (token == "file")
					{
						newSegment.dataSourceType = Segment::DataSourceType::FILE;

						fileIn.getline(buffer, 260);
						token = string(buffer);

						// trim leading and trailing whitespace
						auto first = token.find_first_not_of(" \t");
						auto len = token.find_last_not_of(" \t") - first + 1;
						token = token.substr(first, len);

						newSegment.dataSourceFile = token;
						cout << "\tdata file " << token << endl;
					}
					else if (token == "perlin")
					{
						newSegment.dataSourceType = Segment::DataSourceType::PERLIN;

						fileIn >> newSegment.perlinFrequency >> newSegment.perlinPersistence >> newSegment.amplitude;
						cout << "\tperlin noise " << newSegment.perlinFrequency << " " << newSegment.perlinPersistence << " " << newSegment.amplitude << endl;
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
		_grid = new uint[_width * _height * _depth];
		memset(_grid, 0, sizeof(uint) * _width * _height * _depth);
		uint* ptr;

		// allocate the segment render list and fill it with the segments.
		// move any segments with blend enabled to the end
		_segmentRenderList.alloc(_segments.size());
		uint idx = 0;
		// move non-blended segments to the front
		for (uint i = 0; i < _segments.size(); ++i)
		{
			if (!_segments[i].useBlend)
			{
				_segmentRenderList[idx] = i;
				idx++;
			}
		}
		// and fill in the blended segments at the end
		for (uint i = 0; i < _segments.size(); ++i)
		{
			if (_segments[i].useBlend)
			{
				_segmentRenderList[idx] = i;
				idx++;
			}
		}

		/*
		for (uint z = 0; z < _depth; ++z)
			for (uint y = 0; y < _height; ++y)
				for (uint x = 0; x < _width; ++x)
				{
					uint64 index = x + y * _width + z * _width * _height;

					if (x < 1)
						_grid[index] = voxelIDMap["stone"];
					else if (y < 1)
						_grid[index] = voxelIDMap["soil"];
					else
						_grid[index] = 0;
				}

		return;
		*/

		// now iterate over each new segment, acquire its data and accumulate into the world grid.
		for (uint i = 0; i < _segments.size(); ++i)
		{
			// set start of write pointer to the beginning of the grid
			ptr = _grid;
			
			// load from disk
			if (_segments[i].dataSourceType == Segment::DataSourceType::FILE)
			{
				cout << "loading data for segment " << _segments[i].name << " from " << _segments[i].dataSourceFile << "..." << endl;
			
				ifstream fileIn;
				// 1024 dwords is 4KB of data from disk, ideally an optimal chunk size for sequential reading of large files?
				uint chunk[1024];
				fileIn.open(_segments[i].dataSourceFile, ios::in | ios::binary);
				fileIn.read(reinterpret_cast<char*>(chunk), 4096);

				while (!fileIn.eof() && !fileIn.fail())
				{
					// union the data of this chunk with the grid data.  voxels in the chunk that are zero leave the existing
					// grid data unchanged.
					for (int j = 0; j < 1024; ++j)
					{
						if (chunk[j] == 1)
							*ptr = _segments[i].voxelID;
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
						*ptr = _segments[i].voxelID;
					ptr++;
				}

				fileIn.close();
			}
			else if (_segments[i].dataSourceType == Segment::DataSourceType::PERLIN)
			{
				cout << "generating Perlin noise data for segment " << _segments[i].name << "..." << endl;

				// generate random perlin noise in the XZ direction, with the noise value becoming terrain height.
				// the column of voxels under the top voxel is filled in, so we get a downward fill
				for (uint x = 0; x < _width; ++x)
				{
					for (uint z = 0; z < _depth; ++z)
					{
						// the Perlin noise functions return doubles between [-1.0, 1.0] but the distribution varies with persistence.
						// bias the value to [0.0, 1.0] then scale by grid height
						uint maxY = static_cast<uint>((Perlin::PerlinNoise2D(static_cast<double>(x), static_cast<double>(z), _segments[i].perlinFrequency, _segments[i].perlinPersistence) / 2.0 + 0.5) * static_cast<double>(_height) * _segments[i].amplitude);
						
						// fill the column of voxels up to maxY
						for (uint y = 0; y < maxY; ++y)
						{
							uint64 index = x + y * _width + z * _width * _height;
							_grid[index] = _segments[i].voxelID;
						}			
					}
				}
			}
		}
	}
}

void World::_computeVisibleFaces()
{
	uint v, n;
	// first, determine which solid voxels have exposed faces and compile a list with
	// the integer coords of the voxel and the face flags (bits 0 through 5).  the voxel id
	// of the current voxel is mapped to the segment made of that same id, which then supplies
	// the computeVisibleFaces function with the correct visiblethrough list
	for (uint z = 0; z < _depth; ++z)
	{
		for (uint y = 0; y < _height; ++y)
		{
			for (uint x = 0; x < _width; ++x)
			{
				uint64 index = x + y * _width + z * _width * _height;
				v = _grid[index];

				// only check non-empty voxels
				if (v != 0)
				{
					Segment* s = &_segments[_segmentMap[v]];
					n = _computeVisibleFaces(x, y, z, &s->visibleThroughIDs);

					if (n > 0)
						s->visibleFaceCount += n;
				}
			}
		}
	}

	for (uint i = 0; i < _segments.size(); ++i)
		cout << "segment " << _segments[i].name << " visible faces: " << _segments[i].visibleFaceCount << endl;
}

// checks which of the 6 faces of a voxel are adjacent to an empty voxel or any voxel id
// in the provided array.
uint World::_computeVisibleFaces(uint x, uint y, uint z, Array<uint>* vtids)
{
	// the various offsets used to check adjacent voxels
	static int dx[] = {1,-1,0,0,0,0};
	static int dy[] = {0,0,1,-1,0,0};
	static int dz[] = {0,0,0,0,1,-1};
	int visibleFaceCount = 0;
	uint faceFlags = 0;
	bool isVisible;

	// loop through faces 0 to 5, +x, -x, +y, -y, +z, and -z.
	for (uint face = 0; face < 6; ++face)
	{
		// compute adjacent voxel coord
		int tx = static_cast<int>(x) + dx[face];
		int ty = static_cast<int>(y) + dy[face];
		int tz = static_cast<int>(z) + dz[face];

		isVisible = false;

		// check if the current adjacent voxel coord is out of bounds, which will happen with edge voxels
		if (!((tx < 0) || (tx >= static_cast<int>(_width)) || (ty < 0) || (ty >= static_cast<int>(_height)) || (tz < 0) || (tz >= static_cast<int>(_depth))))
		{
			uint64 index = static_cast<uint>(tx) + static_cast<uint>(ty) * _width + static_cast<uint>(tz) * _width * _height;

			// if the adjacent voxel is empty, then the face is visible
			if (_grid[index] == 0)
				isVisible = true;
			// also ensure there's a list to compare to at all
			else if (vtids->size() > 0)
			{
				// loop through the visiblethrough types and check if the adjacent voxel is any of these types
				for (uint i = 0; i < vtids->size(); ++i)
				{
					if ((_grid[index] & 0x00FFFFFF) == (*vtids)[i])
					{
						isVisible = true;
						break;
					}
				}
			}

			if (isVisible)
			{
				faceFlags |= (1 << face);
				visibleFaceCount++;
			}
		}
	}

	if (faceFlags != 0)
	{
		uint64 index = x + y * _width + z * _width * _height;
		_grid[index] &= 0x00FFFFFF;
		_grid[index] |= (faceFlags << 24);
	}

	return visibleFaceCount;
}

void World::_buildRenderBuffer()
{
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
	
	// next, allocate room for all vertex data that will comprise the visible faces.
	// each vertex has 5 floats: x, y, z, u, and v.  the vertex data will be first, then
	// the uv data.  this is done per segment
	for (uint i = 0; i < _segments.size(); ++i)
	{
		// by design, for now, one giant VBO is used, containing both position and uv data.
		// no index data is used; pure glDrawArrays.
		_segmentRenderData[i].vertexBuffers.alloc(1);

		// 6 verts per face, 5 floats per vert (x,y,z,u,v)
		const int floatsPerFace = 6 * 5;
		_segmentRenderData[i].vertexBuffers[0].size = _segments[i].visibleFaceCount * floatsPerFace * sizeof(float);
		_segmentRenderData[i].vertexBuffers[0].data = new float[_segmentRenderData[i].vertexBuffers[0].size / sizeof(float)];

		// cast to a float pointer and manually populate
		float* ptr = _segmentRenderData[i].vertexBuffers[0].data;
		// ptr2 points to the start of the u,v, which is face-count * 6 * 3 floats from the beginning
		float* ptr2 = _segmentRenderData[i].vertexBuffers[0].data + _segments[i].visibleFaceCount * 6 * 3;

		cout << "segment " << _segments[i].name << endl;
		cout << "\tfacecount " << _segments[i].visibleFaceCount << endl;
		cout << "\tbuffer size " << _segmentRenderData[i].vertexBuffers[0].size << endl;
		
		uint faceCount = 0;
		// then, iterate the visible voxels vector and generate triangle and uv data.
		// this data is just sequential GL_TRIANGLES; no indexed drawing is used.
		for (uint z = 0; z < _depth; ++z)
		{
			for (uint y = 0; y < _height; ++y)
			{
				for (uint x = 0; x < _width; ++x)
				{
					uint64 index = x + y * _width + z * _width * _height;

					// only compute faces for voxels that were generated from the current segment
					if ((_grid[index] & 0x00FFFFFF) == _segments[i].voxelID)
					{
						uint faceFlags = (_grid[index] & 0xFF000000) >> 24;
					
						if (faceFlags != 0)
						{
							// since the cube is 1x1x1 and centered at the origin, bias the data by 0.5 plus the voxel coord
							float dx = static_cast<float>(x) + 0.5f;
							float dy = static_cast<float>(y) + 0.5f;
							float dz = static_cast<float>(z) + 0.5f;

							// loop through each face, 0 to 5
							for (uint face = 0; face < 6; ++face)
							{
								// if that face's bit is set, it's visible
								if ((faceFlags & (1 << face)) != 0)
								{
									faceCount++;
									// loop through the 6 verts of the 2 triangles of the face
									for (uint v = 0; v < 6; ++v)
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
					}
				}
			}
		}

		// define the vertex attributes of the geometry just built
		_segmentRenderData[i].vertexAttribs.alloc(2);

		VertexAttrib va;
		va.name = "pos";
		va.index = 0;
		va.componentCount = 3;
		va.dataType = GL_FLOAT;
		va.stride = 0;
		va.offset = 0;
		_segmentRenderData[i].vertexAttribs[0] = va;

		va.name = "uv0";
		va.index = 3;
		va.componentCount = 2;
		va.dataType = GL_FLOAT;
		va.stride = 0;
		va.offset = reinterpret_cast<void*>(_segments[i].visibleFaceCount * 6 * 3 * sizeof(float));
		_segmentRenderData[i].vertexAttribs[1] = va;

		_segmentRenderData[i].elementCount = _segments[i].visibleFaceCount * 6;

		// finally, create the VAO and VBO, and set vertex attributes
		glGenVertexArrays(1, &_segmentRenderData[i].vao);
		glBindVertexArray(_segmentRenderData[i].vao);

		_segmentRenderData[i].vertexBuffers[0].target = GL_ARRAY_BUFFER;
		_segmentRenderData[i].vertexBuffers[0].usage = GL_STATIC_DRAW;

		glGenBuffers(1, &_segmentRenderData[i].vertexBuffers[0].vbo);
		glBindBuffer(_segmentRenderData[i].vertexBuffers[0].target, _segmentRenderData[i].vertexBuffers[0].vbo);
		glBufferData(_segmentRenderData[i].vertexBuffers[0].target, _segmentRenderData[i].vertexBuffers[0].size, _segmentRenderData[i].vertexBuffers[0].data, _segmentRenderData[i].vertexBuffers[0].usage);

		// enable and set the vertex attribs
		for (uint j = 0; j < _segmentRenderData[i].vertexAttribs.size(); ++j)
		{
			VertexAttrib* va = &_segmentRenderData[i].vertexAttribs[j];
			glEnableVertexAttribArray(va->index);
			glVertexAttribPointer(va->index, va->componentCount, va->dataType, GL_FALSE, va->stride, va->offset);
		}

		// done defining the VAO
		glBindVertexArray(0);
	}
}

void World::render(Shader* shader, bool hideTerrain)
{
	if (!hideTerrain)
	{
		// render all the segments
		shader->setUniform1i("useTexturing", 1);
		//glVertexAttrib4f(1, 1.0f, 0.2f, 0.2f, 1.0f);
		glVertexAttrib4f(1, 0.0f, 1.0f, 0.0f, 1.0f);

		for (uint i = 0; i < _segmentRenderList.size(); ++i)
		{
			uint segmentIdx = _segmentRenderList[i];
			Segment* s = &_segments[segmentIdx];
			RenderData* rd = &_segmentRenderData[segmentIdx];
			//uint id = s->voxelID;
			//Texture* tex = (*_voxelDefs)[id].texture;
			//tex->bind(0);
			(*_voxelDefs)[s->voxelID].texture->bind(0);

			// segments that use blend should be last in the list
			if (s->useBlend)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glBindVertexArray(rd->vao);
				glDrawArrays(GL_TRIANGLES, 0, rd->elementCount);

				glDisable(GL_BLEND);
			}
			/*
			// hack to make soil nicer looking
			else if (s->voxelID == 1)
			{
				shader->setUniform1i("useHeightFactor", 1);
				shader->setUniform1f("worldHeight", static_cast<float>(_height));
				glBindVertexArray(rd->vao);
				glDrawArrays(GL_TRIANGLES, 0, rd->elementCount);
				shader->setUniform1i("useHeightFactor", 0);
			}
			*/
			// regular draw
			else
			{
				glBindVertexArray(rd->vao);
				glDrawArrays(GL_TRIANGLES, 0, rd->elementCount);
			}		
		}
	}
}
