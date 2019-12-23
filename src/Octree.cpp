#include "Octree.h"


Octree::Octree():
	_root(NULL),
	_grid(NULL),
	_gridSize(0), _threshold(0),
	_leafCount(0)
{
	_nodes.reserve(1024);
}


Octree::~Octree()
{
	for (uint i = 0; i < _nodes.size(); ++i)
		delete _nodes[i];

	for (uint i = 0; i < _segmentRenderData.size(); ++i)
	{
		glDeleteVertexArrays(1, &_segmentRenderData[i].vao);
		for (uint j = 0; j < _segmentRenderData[i].vertexBuffers.size(); ++j)
		{
			glDeleteBuffers(1, &_segmentRenderData[i].vertexBuffers[j].vbo);
			if (_segmentRenderData[i].vertexBuffers[j].data != NULL)
				delete [] _segmentRenderData[i].vertexBuffers[j].data;
		}

		for (uint j = 0; j < _segmentRenderData[i].indexBuffers.size(); ++j)
		{
			glDeleteBuffers(1, &_segmentRenderData[i].indexBuffers[j].ibo);
			if (_segmentRenderData[i].indexBuffers[j].data != NULL)
				delete [] _segmentRenderData[i].indexBuffers[j].data;
		}
	}
}

void Octree::build(uint* grid, uint gridSize, Array<Segment>* segments, Array<uint>* segmentMap, Array<Voxel>* voxelDefs, Array<uint>* segmentRenderList, uint threshold)
{
	MatrixStack m;

	cout << "starting octree build..." << endl;
	
	// save all of the world pointers and parameters.  needed during recursion and build functions
	_grid = grid;
	_gridSize = gridSize;
	_segments = segments;
	_segmentMap = segmentMap;
	_voxelDefs = voxelDefs;
	_segmentRenderList = segmentRenderList;

	// initialize arrays holding render data, 1 index per segment
	_segmentRenderData.alloc(_segments->size());
	_posPtrs.alloc(_segments->size());
	_uvPtrs.alloc(_segments->size());

	// setup temp float buffer and pointers per segment
	for (uint i = 0; i < _segmentRenderData.size(); ++i)
	{
		// 1 VBO per segment, holding both pos and uv data
		_segmentRenderData[i].vertexBuffers.alloc(1);
		// allocate temporary float buffer to hold all of the terrain geometry as it's built,
		// piece by piece, during tree-build recursion.  6 verts per face, 5 floats per vert (3 pos + 2 uv)
		_segmentRenderData[i].vertexBuffers[0].data = new float[(*_segments)[i].visibleFaceCount * 6 * 5];
		_segmentRenderData[i].vertexBuffers[0].size = (*_segments)[i].visibleFaceCount * 6 * 5 * sizeof(float);
		
		// initialize the actual pointers that _buildLeafBuffer will use
		_posPtrs[i] = _segmentRenderData[i].vertexBuffers[0].data;
		// skip the vert data (faces * 6 * 3) and start at uv data
		_uvPtrs[i] = _segmentRenderData[i].vertexBuffers[0].data + (*_segments)[i].visibleFaceCount * 6 * 3;
	}
	
	// construct the root node using the full world grid
	Node* node;
	node = new Node;
	node->parent = NULL;
	memset(node->child, 0, sizeof(Node*) * 8);
	node->isLeaf = false;
	node->isEmpty = false;
	node->depth = 0;
	memset(&node->neighbors[0], 0, sizeof(Node*) * 6);

	// set dimensions
	node->x = 0;
	node->y = 0;
	node->z = 0;
	node->size = gridSize;
	node->box.set(Vector3(static_cast<float>(node->x), static_cast<float>(node->y), static_cast<float>(node->z)), static_cast<float>(node->size), static_cast<float>(node->size), static_cast<float>(node->size));
	node->color = Vector3(0.0f, 0.0f, 1.0f);

	m.makeIdentity();
	// transforms a box defined from [-0.5,-0.5,-0.5] to [0.5,0.5,0.5] into a box defined from [x,y,z] to [x+size,y+size,z+size]
	m.translate(static_cast<float>(gridSize) / 2.0f, static_cast<float>(gridSize) / 2.0f, static_cast<float>(gridSize) / 2.0f);
	m.scale(static_cast<float>(gridSize), static_cast<float>(gridSize), static_cast<float>(gridSize));
	node->wireframeWorldMatrix = m.top();
	
	// zero these out for now?
	node->visibleFaceCount = 0;
	//node->elementCounts.alloc(_segments->size());
	//node->offsets.alloc(_segments->size());

	// next, recurse over the grid and build the tree, with leaves being chunks of the set of visible faces.
	// the threshold determines the count at which the node is made into a leaf.
	cout << "building tree..." << endl;

	// anchor the tree
	_root = node;
	_nodes.push_back(_root);
	_threshold = threshold;
	_treeDepth = 0;
	_emptyLeafCount = 0;

	// engage!
	_buildRecurse(_root);

	cout << "computing adjacency info..." << endl;
	_computeAdjacencyInfo();

	cout << "building render buffers... (" << glGetError() << ")" << endl;
	_buildRenderData();
	cout << "done. (" << glGetError() << ")" << endl << endl;
	
	cout << "total nodes " << _nodes.size() << endl;
	cout << "tree depth " << _treeDepth << endl;
	cout << "leafcount " << _leafCount << endl;
	cout << "empty leaves " << _emptyLeafCount << endl;
	cout << "approximate system memory usage " << static_cast<float>(_nodes.size() * sizeof(Node)) / 1048576.0f << "MB" << endl;
	//cout << "approximate video memory usage " << static_cast<float>(_nodes.size() * (24 * sizeof(float) + 16 * sizeof(unsigned short))) / 1048576.0f << "MB" << endl;

	uint totalFaceCount = 0;
	for (uint i = 0; i < _nodes.size(); ++i)
		for (uint j = 0; j < _nodes[i]->elementCounts.size(); ++j)
			totalFaceCount += (_nodes[i]->elementCounts[j] / 6);
	cout << "total faces across all leaves " << totalFaceCount << endl;
}

void Octree::_buildRecurse(Node* node)
{
	MatrixStack m;

	if (node != NULL)
	{
		// find the number of visible faces in this node
		node->visibleFaceCount = _countVisibleFaces(node);

		// if the count of visible faces is below the threshold, the node becomes a leaf node.
		// if the node has shrunk to 1 voxel, it becomes a leaf as well
		if ((node->visibleFaceCount < _threshold) || (node->size == 1))
		{
			node->isLeaf = true;
			_leafCount++;

			// if this leaf isn't empty, build the geometry it contains
			if (node->visibleFaceCount > 0)
			{
				node->isEmpty = false;
				node->color = Vector3(0.0f, 1.0f, 0.0f);
				_buildLeafBuffer(node);
			}
			else
			{
				node->isEmpty = true;
				node->color = Vector3(0.5f, 0.5f, 0.5f);
				_emptyLeafCount++;
			}

			// track total tree depth
			if (node->depth > _treeDepth)
				_treeDepth = node->depth;
		}
		// else, split into 8 and recurse!
		else
		{
			// used for computing the child node start positions
			static unsigned char dx[] = {0,0,0,0,1,1,1,1};
			static unsigned char dy[] = {0,0,1,1,0,0,1,1};
			static unsigned char dz[] = {0,1,0,1,0,1,0,1};

			// initialize a new child and recurse
			for (int i = 0; i < 8; ++i)
			{
				Node* child;
				child = new Node;

				// set pointers
				node->child[i] = child;
				child->parent = node;
				memset(child->child, 0, sizeof(Node*) * 8);
				child->isLeaf = false;
				child->isEmpty = false;
				child->depth = node->depth + 1;
	
				// compute new dimensions
				child->x = node->x + dx[i] * node->size / 2;
				child->y = node->y + dy[i] * node->size / 2;
				child->z = node->z + dz[i] * node->size / 2;
				child->size = node->size / 2;
				child->color = Vector3(0.514f, 0.318f, 0.212f);

				// setup AABB
				child->box.set(Vector3(static_cast<float>(child->x), static_cast<float>(child->y), static_cast<float>(child->z)), static_cast<float>(child->size), static_cast<float>(child->size), static_cast<float>(child->size));
				// setup world transform used to render the wireframe box
				m.makeIdentity();
				m.translate(static_cast<float>(child->x) + static_cast<float>(child->size) / 2.0f, static_cast<float>(child->y) + static_cast<float>(child->size) / 2.0f, static_cast<float>(child->z) + static_cast<float>(child->size) / 2.0f);
				m.scale(static_cast<float>(child->size), static_cast<float>(child->size), static_cast<float>(child->size));
				child->wireframeWorldMatrix = m.top();

				_nodes.push_back(child);

				// recurse zee children!
				_buildRecurse(child);
			}
		}
	}
	else
		cout << "ERROR: _buildRecurse called with a NULL node! not cool." << endl;
}

uint Octree::_countVisibleFaces(Node* node)
{
	uint faceCount = 0;

	if (node != NULL)
	{
		// loop through the node's region of the grid and count the visible faces
		uint xMax = node->x + node->size;
		uint yMax = node->y + node->size;
		uint zMax = node->z + node->size;
			
		for (uint z = node->z; z < zMax; ++z)
		{
			for (uint y = node->y; y < yMax; ++y)
			{
				for (uint x = node->x; x < xMax; ++x)
				{
					uint64 index = x + y * _gridSize + z * _gridSize * _gridSize;
					unsigned char flags = (_grid[index] & 0xFF000000) >> 24;
					faceCount += _countBits(flags);
				}
			}
		}

		return faceCount;
	}
	else
	{
		cout << "ERROR: _countVisibleFaces called with a NULL node! not cool." << endl;
		return 0;
	}
}

uint Octree::_countBits(unsigned char b)
{
	// lazy way of mapping a byte to its length in 1's
	static uint byteToBitCount[] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
											1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
											1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
											2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
											1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
											2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
											2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
											3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8};
	return byteToBitCount[b];
}

void Octree::_buildLeafBuffer(Node* node)
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

	uint v, faceFlags;
	float* posPtr;
	float* uvPtr;

	// initialize the node's elementCount and offset arrays
	node->elementCounts.alloc(_segments->size());
	node->elementCounts.clear();
	node->offsets.alloc(_segments->size());
	node->offsets.clear();

	// first, compute the offset to start drawing at in the giant VBO for this node.  the difference between the current
	// position of the "traveling" float pointer for this segment and the start of the buffer gives the number of floats
	// they differ by.  dividing by 3 gives the number of verts to offset by when doing glDrawArrays.
	for (uint i = 0; i < _segments->size(); ++i)
		node->offsets[i] = static_cast<uint>(_posPtrs[i] - _segmentRenderData[i].vertexBuffers[0].data) / 3;

	// next, loop through the whole grid and build triangles
	for (uint z = node->z; z < (node->z + node->size); ++z)
	{
		for (uint y = node->y; y < (node->y + node->size); ++y)
		{
			for (uint x = node->x; x < (node->x + node->size); ++x)
			{
				uint64 index = x + y * _gridSize + z * _gridSize * _gridSize;
				v = _grid[index];
				
				// since the cube is 1x1x1 and centered at the origin, bias the data by 0.5 plus the voxel coord
				float dx = static_cast<float>(x) + 0.5f;
				float dy = static_cast<float>(y) + 0.5f;
				float dz = static_cast<float>(z) + 0.5f;

				// only check non-empty voxels
				if (v != 0)
				{
					// check visible face count
					faceFlags = (v & 0xFF000000) >> 24;
					if (faceFlags != 0)
					{
						// map the voxel to a segment to retrieve the corresponding renderdata's buffer pointers
						uint segmentIdx = (*_segmentMap)[v & 0x00FFFFFF];
						posPtr = _posPtrs[segmentIdx];
						uvPtr = _uvPtrs[segmentIdx];

						// increment the elementCount for the corresponding segment based on the face count in this voxel
						node->elementCounts[segmentIdx] += (_countBits(faceFlags) * 6);

						// loop through the faces
						for (uint face = 0; face < 6; ++face)
						{
							// if current face is visible
							if (faceFlags & (1 << face))
							{
								// loop through the 6 verts of the 2 triangles of the face
								for (uint v = 0; v < 6; ++v)
								{
									// offset the object-space indexed cube vertex by the current voxel position					
									*posPtr = cubeVertexData[cubeIndexData[face * 6 + v] * 3 + 0] + dx; posPtr++;
									*posPtr = cubeVertexData[cubeIndexData[face * 6 + v] * 3 + 1] + dy; posPtr++;
									*posPtr = cubeVertexData[cubeIndexData[face * 6 + v] * 3 + 2] + dz; posPtr++;
									*uvPtr = cubeUVData[v * 2 + 0]; uvPtr++;
									*uvPtr = cubeUVData[v * 2 + 1]; uvPtr++;
								}
							}
						}

						// update the class-wide pointers to the pos and uv data, so the next node's data will be filled in
						// right after the data of this node.
						_posPtrs[segmentIdx] = posPtr;
						_uvPtrs[segmentIdx] = uvPtr;
					}
				}
			}
		}
	}
}

// compute adjacency info for each leaf node
void Octree::_computeAdjacencyInfo()
{
	for (uint i = 0; i < _nodes.size(); ++i)
	{
		if (_nodes[i]->isLeaf)
		{
			int x = static_cast<int>(_nodes[i]->x);
			int y = static_cast<int>(_nodes[i]->y);
			int z = static_cast<int>(_nodes[i]->z);
			int size = static_cast<int>(_nodes[i]->size);
			int gsize = static_cast<int>(_gridSize);
			uint depth = _nodes[i]->depth;

			int dx, dy, dz;

			for (uint j = 0; j < 6; ++j)
			{
				// find neighbors in order: +X, -X, +Y, -Y, +Z, -Z.
				// yet another random method to loopify all these repetitive but slightly dissimilar calcs
				switch (j)
				{
					case 0: dx = x + size + size / 2; dy = y; dz = z; break;
					case 1: dx = x - size / 2; dy = y; dz = z; break;
					case 2: dx = x; dy = y + size + size / 2; dz = z; break;
					case 3: dx = x; dy = y - size / 2; dz = z; break;
					case 4: dx = x; dy = y; dz = z + size + size / 2; break;
					case 5: dx = x; dy = y; dz = z - size / 2; break;
				};

				// make sure the target voxel is within the bounds of the octree as a whole.
				// the findNode function really should be doing this......
				if ((dx >= 0 && dx < gsize) && (dy >= 0 && dy < gsize) && (dz >= 0 && dz < gsize))
					_nodes[i]->neighbors[j] = _findNode(static_cast<uint>(dx), static_cast<uint>(dy), static_cast<uint>(dz), depth);
				else
					_nodes[i]->neighbors[j] = NULL;
			}
		}
		// "branch" nodes don't get this info.  they're not special enough (for now).
		else
			memset(&_nodes[i]->neighbors[0], 0, sizeof(Node*) * 6);
	}
}

void Octree::_buildRenderData()
{
	// build the wireframe buffer used for wireframe rendering
	_buildWireframeBuffer();
	cout << "done with wireframe render data (" << glGetError() << ")" << endl;
	cout << "building leaf render data..." << endl;

	// build 1 VAO/VBO per segment
	for (uint i = 0; i < _segmentRenderData.size(); ++i)
	{
		// define the vertex attributes of the geometry built in the leaves
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
		va.offset = reinterpret_cast<void*>((*_segments)[i].visibleFaceCount * 6 * 3 * sizeof(float));
		_segmentRenderData[i].vertexAttribs[1] = va;

		glGetError();

		// create VAO
		glGenVertexArrays(1, &_segmentRenderData[i].vao);
		CHECKGLERROR(glGenVertexArrays)
		glBindVertexArray(_segmentRenderData[i].vao);
		CHECKGLERROR(glBindVertexArray)

		// create VBO
		_segmentRenderData[i].vertexBuffers[0].target = GL_ARRAY_BUFFER;
		_segmentRenderData[i].vertexBuffers[0].usage = GL_STATIC_DRAW;

		glGenBuffers(1, &_segmentRenderData[i].vertexBuffers[0].vbo);
		CHECKGLERROR(glGenBuffers)
		glBindBuffer(_segmentRenderData[i].vertexBuffers[0].target, _segmentRenderData[i].vertexBuffers[0].vbo);
		CHECKGLERROR(glBindBuffer)
		glBufferData(_segmentRenderData[i].vertexBuffers[0].target, _segmentRenderData[i].vertexBuffers[0].size, _segmentRenderData[i].vertexBuffers[0].data, _segmentRenderData[i].vertexBuffers[0].usage);
		CHECKGLERROR(glBufferData)

		// set vertex attrib parameters
		for (uint j = 0; j < _segmentRenderData[i].vertexAttribs.size(); ++j)
		{
			VertexAttrib* va = &_segmentRenderData[i].vertexAttribs[j];
			glEnableVertexAttribArray(va->index);
			CHECKGLERROR(glEnableVertexAttribArray)
			glVertexAttribPointer(va->index, va->componentCount, va->dataType, GL_FALSE, va->stride, va->offset);
			CHECKGLERROR(glVertexAttribPointer)
		}

		glBindVertexArray(0);
		CHECKGLERROR(glBindVertexArray)
	}
}

Octree::Node* Octree::_findNode(uint x, uint y, uint z, uint depth)
{
	// make sure there's a tree and some nodes to work with
	if (_root != NULL && _nodes.size() > 0)
	{
		return _findNodeRecurse(_root, x, y, z, depth);
	}
	else
		return NULL;
}

Octree::Node* Octree::_findNodeRecurse(Node* node, uint x, uint y, uint z, uint depth)
{
	// first check whether this node is at the depth limit
	if (node->depth == depth)
		return node;
	else
	{
		// if this is a leaf node, we're still returning since there
		// isn't any deeper we could recurse.
		if (node->isLeaf)
			return node;
		else
		{
			// otherwise, find the 1 child node that contains the target voxel and recurse
			for (uint i = 0; i < 8; ++i)
			{
				uint size = node->child[i]->size;
				if ((x >= node->child[i]->x && x < (node->child[i]->x + size)) &&
					(y >= node->child[i]->y && y < (node->child[i]->y + size)) &&
					(z >= node->child[i]->z && z < (node->child[i]->z + size)))
				{
					return _findNodeRecurse(node->child[i], x, y, z, depth);
				}
			}
		}
	}

	// this shouldn't happen!  I probably should redesign things >_>
	return NULL;
}

void Octree::_buildWireframeBuffer()
{
	// 8 vertices, 3 floats per vertex (x,y,z)
	static float wireframeVertexData[8 * 3] = {-0.5f, -0.5f, -0.5f,
											   -0.5f, -0.5f,  0.5f,
											   -0.5f,  0.5f, -0.5f,
											   -0.5f,  0.5f,  0.5f,
											    0.5f, -0.5f, -0.5f,
											    0.5f, -0.5f,  0.5f,
											    0.5f,  0.5f, -0.5f,
											    0.5f,  0.5f,  0.5f};
	// this draws 2 halves of a wireframe cube using a line strip.  14 lines, 2 degenerate, not bad!
	static unsigned short wireframeIndexData[] = {7,3,2,6,4,5,7,6,  1,3,2,0,4,5,1,0};

	// only 1 vertex attribute: position
	_wireframeRenderData.vertexAttribs.alloc(1);

	VertexAttrib va;
	va.name = "pos";
	va.componentCount = 3;
	va.dataType = GL_FLOAT;
	va.index = 0;
	va.offset = 0;
	va.stride = 0;

	_wireframeRenderData.vertexAttribs[0] = va;

	// generate VAO
	glGenVertexArrays(1, &_wireframeRenderData.vao);
	glBindVertexArray(_wireframeRenderData.vao);

	// generate 1 VBO
	_wireframeRenderData.vertexBuffers.alloc(1);
	_wireframeRenderData.vertexBuffers[0].target = GL_ARRAY_BUFFER;
	_wireframeRenderData.vertexBuffers[0].usage = GL_STATIC_DRAW;
	
	glGenBuffers(1, &_wireframeRenderData.vertexBuffers[0].vbo);
	glBindBuffer(_wireframeRenderData.vertexBuffers[0].target, _wireframeRenderData.vertexBuffers[0].vbo);
	glBufferData(_wireframeRenderData.vertexBuffers[0].target, 8 * 3 * sizeof(float), wireframeVertexData, _wireframeRenderData.vertexBuffers[0].usage);

	// generate 1 IBO
	_wireframeRenderData.indexBuffers.alloc(1);
	_wireframeRenderData.indexBuffers[0].target = GL_ELEMENT_ARRAY_BUFFER;
	_wireframeRenderData.indexBuffers[0].usage = GL_STATIC_DRAW;
	_wireframeRenderData.indexBuffers[0].dataType = GL_UNSIGNED_SHORT;
	
	glGenBuffers(1, &_wireframeRenderData.indexBuffers[0].ibo);
	glBindBuffer(_wireframeRenderData.indexBuffers[0].target, _wireframeRenderData.indexBuffers[0].ibo);
	glBufferData(_wireframeRenderData.indexBuffers[0].target, 16 * sizeof(unsigned short), wireframeIndexData, _wireframeRenderData.indexBuffers[0].usage);

	// setup vertex attrib
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, _wireframeRenderData.vertexAttribs[0].componentCount, _wireframeRenderData.vertexAttribs[0].dataType, GL_FALSE, _wireframeRenderData.vertexAttribs[0].stride, _wireframeRenderData.vertexAttribs[0].offset);
	
	// done!
	glBindVertexArray(0);
}

// no-blend rendering of segments 
void Octree::render(Shader* shader, const Matrix4& projViewMatrix)
{
	shader->setUniform1i("useTexturing", 1);
	glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);
	
	// loop through each segment's render data
	for (uint i = 0; i < _segmentRenderData.size(); ++i)
	{
		// bind the texture and VAO
		(*_voxelDefs)[(*_segments)[i].voxelID].texture->bind(0);
		glBindVertexArray(_segmentRenderData[i].vao);
		
		// loop through the leaf nodes and render their portion of this segment
		for (uint j = 0; j < _nodes.size(); ++j)
		{
			if (_nodes[j]->isLeaf && !_nodes[j]->isEmpty)
				if (_nodes[j]->elementCounts[i] > 0)
					glDrawArrays(GL_TRIANGLES, _nodes[j]->offsets[i], _nodes[j]->elementCounts[i]);
		}
	}

	shader->setUniform1i("useTexturing", 0);
}

// hardcoded to draw the first segment's entire VBO as one draw call, no tree recursion
void Octree::render2(Shader* shader, const Matrix4& projViewMatrix)
{
	shader->setUniform1i("useTexturing", 1);
	glVertexAttrib4f(1, 0.2f, 0.2f, 0.2f, 1.0f);
	(*_voxelDefs)[(*_segments)[0].voxelID].texture->bind(0);
	glBindVertexArray(_segmentRenderData[0].vao);
	glDrawArrays(GL_TRIANGLES, 0, (*_segments)[0].visibleFaceCount * 6);
	shader->setUniform1i("useTexturing", 0);
}

// renders segments in blend-sorted order, but no tree traversal, just iterates the node list
void Octree::render3(Shader* shader, const Matrix4& projViewMatrix)
{
	shader->setUniform1i("useTexturing", 1);
	glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);
	
	// loop through each segment's render data
	for (uint i = 0; i < _segmentRenderList->size(); ++i)
	{
		uint segmentIdx = (*_segmentRenderList)[i];
		// bind the texture and VAO
		(*_voxelDefs)[(*_segments)[segmentIdx].voxelID].texture->bind(0);
		glBindVertexArray(_segmentRenderData[segmentIdx].vao);

		if ((*_segments)[segmentIdx].useBlend)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// loop through the leaf nodes and render their portion of this segment
			for (uint j = 0; j < _nodes.size(); ++j)
			{
				if (_nodes[j]->isLeaf && !_nodes[j]->isEmpty)
					if (_nodes[j]->elementCounts[segmentIdx] > 0)
						glDrawArrays(GL_TRIANGLES, _nodes[j]->offsets[segmentIdx], _nodes[j]->elementCounts[segmentIdx]);
			}

			glDisable(GL_BLEND);
		}
		// such hax
		else if ((*_segments)[segmentIdx].voxelID == 1)
		{
			shader->setUniform1i("useHeightFactor", 1);
			shader->setUniform1f("worldHeight", static_cast<float>(_root->size));

			// loop through the leaf nodes and render their portion of this segment
			for (uint j = 0; j < _nodes.size(); ++j)
			{
				if (_nodes[j]->isLeaf && !_nodes[j]->isEmpty)
					if (_nodes[j]->elementCounts[segmentIdx] > 0)
						glDrawArrays(GL_TRIANGLES, _nodes[j]->offsets[segmentIdx], _nodes[j]->elementCounts[segmentIdx]);
			}
			shader->setUniform1i("useHeightFactor", 0);
		}
		else
		{
			// loop through the leaf nodes and render their portion of this segment
			for (uint j = 0; j < _nodes.size(); ++j)
			{
				if (_nodes[j]->isLeaf && !_nodes[j]->isEmpty)
					if (_nodes[j]->elementCounts[segmentIdx] > 0)
						glDrawArrays(GL_TRIANGLES, _nodes[j]->offsets[segmentIdx], _nodes[j]->elementCounts[segmentIdx]);
			}
		}
	}

	shader->setUniform1i("useTexturing", 0);
}

// renders in blend-sorted order, depth-first traversing the octree and frustum culling leaves based on their AABB
void Octree::render4(Shader* shader, Frustum* frustum)
{
	_lastFrameLeafRenderCount = 0;

	shader->setUniform1i("useTexturing", 1);
	glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);

	// loop through each segment, traversing the tree each time
	for (uint i = 0; i < _segmentRenderList->size(); ++i)
	{
		uint segmentIdx = (*_segmentRenderList)[i];
		// bind the texture and VAO
		(*_voxelDefs)[(*_segments)[segmentIdx].voxelID].texture->bind(0);
		glBindVertexArray(_segmentRenderData[segmentIdx].vao);

		if ((*_segments)[segmentIdx].useBlend)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			_render4Recurse(_root, segmentIdx, frustum);

			glDisable(GL_BLEND);
		}
		// such hax.  vary the brightness of the grass voxels based on y-height
		else if ((*_segments)[segmentIdx].voxelID == 1)
		{
			shader->setUniform1i("useHeightFactor", 1);
			shader->setUniform1f("worldHeight", static_cast<float>(_root->size));

			_render4Recurse(_root, segmentIdx, frustum);

			shader->setUniform1i("useHeightFactor", 0);
		}
		else
		{
			_render4Recurse(_root, segmentIdx, frustum);
		}
	}

	shader->setUniform1i("useTexturing", 0);
}

void Octree::_render4Recurse(Node* node, uint segmentIdx, Frustum* frustum)
{
	// if the node contains no geometry, screw it.
	if (!node->isEmpty)
	{
		// if this node's AABox intersects or resides within the frustum, then recurse through it
		if (frustum->containsAABB(node->box) != 0)
		{
			// only leaves contain geometry to render, though some leaves are empty
			if (node->isLeaf)
			{
				// the leaf may only have 1 type of voxel, so depending on the segment being
				// rendered, it may or may not have any geometry to draw
				if (node->elementCounts[segmentIdx] > 0)
				{
					glDrawArrays(GL_TRIANGLES, node->offsets[segmentIdx], node->elementCounts[segmentIdx]);
					_lastFrameLeafRenderCount++;
				}
			}
			// else, recurse
			else
			{
				// loop through the children
				for (uint i = 0; i < 8; ++i)
				{
					//if (node->child[i] != NULL)
					_render4Recurse(node->child[i], segmentIdx, frustum);
				}
			}
		}
	}
}

// render wireframe boxes for the leaf nodes
void Octree::renderWireframes(Shader* shader, const Matrix4& projViewMatrix, Frustum* frustum)
{
	Matrix4 wvp;

	glLineWidth(1.0f);
	shader->setUniform1i("useTexturing", 0);
	glBindVertexArray(_wireframeRenderData.vao);

	for (uint i = 0; i < _nodes.size(); ++i)
	{
		if (_nodes[i]->isLeaf)
		{
			glVertexAttrib3f(1, _nodes[i]->color.x, _nodes[i]->color.y, _nodes[i]->color.z);
			wvp = projViewMatrix * _nodes[i]->wireframeWorldMatrix;
			shader->setMatrix4f("WVPMatrix", wvp);
			shader->setMatrix4f("worldMatrix", _nodes[i]->wireframeWorldMatrix);

			
			glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, 0);
			glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(unsigned short)));
		}
	}

	glBindVertexArray(0);
	//glLineWidth(1.0f);
}

// find a leaf node containing the point and at a depth <= maxDepth
void Octree::renderNodeTest(Shader* shader, const Matrix4& projViewMatrix, Frustum* frustum, const Vector3& testPoint, uint maxDepth)
{
	Node* node;
	float fsize = static_cast<float>(_gridSize);

	if ((testPoint.x > 0.0f && testPoint.x <= fsize) &&
		(testPoint.y > 0.0f && testPoint.y <= fsize) &&
		(testPoint.z > 0.0f && testPoint.z <= fsize))
	{
		node = _findNode(static_cast<uint>(testPoint.x), static_cast<uint>(testPoint.y), static_cast<uint>(testPoint.z), maxDepth);
	}
	else
		node = NULL;

	glLineWidth(3.0f);
	shader->setUniform1i("useTexturing", 0);
	glBindVertexArray(_wireframeRenderData.vao);

	Matrix4 wvp;

	// either use the node's color
	if (node != NULL)
		glVertexAttrib3f(1, node->color.x, node->color.y, node->color.z);
	// or render the root of the octree's node in blue if no node was found
	else
	{
		node = _root;
		glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);
	}

	wvp = projViewMatrix * node->wireframeWorldMatrix;
	shader->setMatrix4f("WVPMatrix", wvp);
	shader->setMatrix4f("worldMatrix", node->wireframeWorldMatrix);

	glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(unsigned short)));	

	glBindVertexArray(0);
	glLineWidth(1.0f);
}

// renders the current node plus its adjacent neighboring nodes
void Octree::renderNodeTest2(Shader* shader, const Matrix4& projViewMatrix, Frustum* frustum, Octree::Node* node, Octree::Node* highlight)
{
	Matrix4 wvp;

	if (node != NULL)
	{
		// draw the target node
		glLineWidth(3.0f);
		glVertexAttrib3f(1, node->color.x, node->color.y, node->color.z);

		// enable stencil test, and draw the current node into the stencil buffer with a value of 1
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		shader->setUniform1i("useTexturing", 0);
		wvp = projViewMatrix * node->wireframeWorldMatrix;
		shader->setMatrix4f("WVPMatrix", wvp);
		shader->setMatrix4f("worldMatrix", node->wireframeWorldMatrix);

		glBindVertexArray(_wireframeRenderData.vao);

		glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, 0);
		glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(unsigned short)));	

		// draw the neighboring nodes in a dark grey, but only ones that don't overlap the stenciled main node as drawn above
		glLineWidth(2.0f);
		glVertexAttrib3f(1, 0.1f, 0.1f, 0.1f);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		for (uint i = 0; i < 6; ++i)
		{
			Node* neighbor = node->neighbors[i];

			if (neighbor != NULL)
			{
				wvp = projViewMatrix * neighbor->wireframeWorldMatrix;
				shader->setMatrix4f("WVPMatrix", wvp);
				shader->setMatrix4f("worldMatrix", neighbor->wireframeWorldMatrix);
			
				glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, 0);
				glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(unsigned short)));
			}
		}

		// draw the children as well, if present
		if (!node->isLeaf)
		{
			for (uint i = 0; i < 8; ++i)
			{
				Node* child = node->child[i];
				wvp = projViewMatrix * child->wireframeWorldMatrix;
				shader->setMatrix4f("WVPMatrix", wvp);
				shader->setMatrix4f("worldMatrix", child->wireframeWorldMatrix);

				glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, 0);
				glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(unsigned short)));
			}
		}

		glBindVertexArray(0);
		glLineWidth(1.0f);
		glDisable(GL_STENCIL_TEST);
	}
}