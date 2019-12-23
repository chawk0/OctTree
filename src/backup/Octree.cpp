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
	for (auto i = _nodes.begin(); i != _nodes.end(); ++i)
		delete *i;
}

void Octree::build(unsigned int* grid, unsigned int gridSize, unsigned int threshold)
{
	cout << "starting octree build..." << endl;
	
	// construct the root node using the full world grid
	Node* node;
	node = new Node;
	node->parent = NULL;
	memset(node->child, 0, sizeof(Node*) * 8);
	node->isLeaf = false;
	
	// save dimensions to build wireframe
	node->x = 0;
	node->y = 0;
	node->z = 0;
	node->size = gridSize;
	_buildWireframeBuffer(node);
	node->color = Vector3(0.0f, 0.0f, 1.0f);
	
	// NULL out the leaf VBO if/until needed
	memset(&node->leafBuffer, 0, sizeof(Buffer));
	node->leafVAO = 0;
	node->leafVBO = 0;

	// save the world grid info to the class so it's reachable during recursion
	_grid = grid;
	_gridSize = gridSize;

	// compute all visible faces for the entire grid.  store the bitflags in the upper byte of
	// the grid value, and store the total face count in the root node.  the bitflags allow the 
	// recursive function to count visible faces in sub-regions fairly easily.
	cout  << "finding visible faces..." << endl;
	node->faceCount = _computeVisibleFaces();
	cout << "done!" << endl;

	// next, recurse over the grid and build the tree, with leaves being chunks of the set of visible faces.
	// the threshold determines the count at which the node is made into a leaf.  these leaf nodes each store a
	// VBO containing that chunk of the visible faces of the landscape.

	cout << "building tree..." << endl;
	// anchor the tree
	_root = node;
	// save all nodes generated for easy cleanup in dtor
	_nodes.push_back(_root);
	// needed while recursing
	_threshold = threshold;
	// engage!
	_buildRecurse(_root);

	cout << "done!" << endl;
	cout << "leafcount " << _leafCount << endl;
	cout << "total nodes " << _nodes.size() << endl;
	cout << "approximate system memory usage " << static_cast<float>(_nodes.size() * sizeof(Node)) / 1048576.0f << "MB" << endl;
	cout << "approximate video memory usage " << static_cast<float>(_nodes.size() * (24 * sizeof(float) + 16 * sizeof(unsigned short))) / 1048576.0f << "MB" << endl;
}

bool Octree::_buildRecurse(Node* node)
{
	if (node != NULL)
	{
		// find the number of visible faces in this node
		node->faceCount = _countVisibleFaces(node);

		// empty children nodes return false so the parent deletes them
		if (node->faceCount == 0)
		{
			return false;
		}
		// if the count of visible faces is below the threshold, the node becomes a leaf node.
		// if the node has shrunk to 1 voxel, it becomes a leaf as well
		else if ((node->faceCount < _threshold) || (node->size == 1))
		{
			node->isLeaf = true;
			node->color = Vector3(0.0f, 1.0f, 0.0f);
			_buildLeafBuffer(node);
			return true;
		}
		// else, split into 8 and recurse!
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
	
				// compute new dimensions
				child->x = node->x + dx[i] * node->size / 2;
				child->y = node->y + dy[i] * node->size / 2;
				child->z = node->z + dz[i] * node->size / 2;
				child->size = node->size / 2;

				// NULL out the leaf VBO if/until needed
				memset(&child->leafBuffer, 0, sizeof(Buffer));
				child->leafVAO = 0;
				child->leafVBO = 0;

				// make this child's wireframe bounding box
				_buildWireframeBuffer(child);					

				// if the recursion from the child node is false, then it was empty
				// so delete it.
				if (!_buildRecurse(child))
				{
					delete node->child[i];
					node->child[i] = NULL;
				}
				// else save this child for future deletion
				else
					_nodes.push_back(child);
			}

			return true;
		}
	}
	else
	{
		cout << "ERROR: _buildRecurse called with a NULL node! not cool." << endl;
		return false;
	}
}

void Octree::_buildWireframeBuffer(Node* node)
{
	//cout << "building node wireframe buffer..." << endl;
	// build a wire frame to render the node visible.
	// these arrays are used to compute the corners
	static unsigned char dx[] = {0,0,0,0,1,1,1,1};
	static unsigned char dy[] = {0,0,1,1,0,0,1,1};
	static unsigned char dz[] = {0,1,0,1,0,1,0,1};

	// 8 vertices, 3 floats per vertex (x,y,z)
	// the position data will be recomputed for each node
	static float wireframeVertexData[8 * 3];
	// this draws 2 halves of a wireframe cube using a line strip.  14 lines, 2 degenerate
	static unsigned short wireframeIndexData[] = {7,3,2,6,4,5,7,6,  1,3,2,0,4,5,1,0};

	// compute the position data for the wireframe lines relative to the node's position.
	for (int i = 0; i < 8; ++i)
	{
		wireframeVertexData[i * 3 + 0] = static_cast<float>(node->x + dx[i] * node->size);
		wireframeVertexData[i * 3 + 1] = static_cast<float>(node->y + dy[i] * node->size);
		wireframeVertexData[i * 3 + 2] = static_cast<float>(node->z + dz[i] * node->size);
	}

	glGenVertexArrays(1, &node->wireframeVAO);
	glBindVertexArray(node->wireframeVAO);

	// create the vertex buffer
	glGenBuffers(1, &node->wireframeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, node->wireframeVBO);
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), wireframeVertexData, GL_STATIC_DRAW);

	// stream 0 is position data, 3 floats
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// create the index buffer
	glGenBuffers(1, &node->wireframeIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, node->wireframeIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 16 * sizeof(unsigned short), wireframeIndexData, GL_STATIC_DRAW);

	// done defining the VAO
	glBindVertexArray(0);

	//cout << "done!" << endl;
}

unsigned int Octree::_countVisibleFaces(Node* node)
{
	unsigned int faceCount = 0;

	if (node != NULL)
	{
		// a small cheat to avoid extra logic.  the root node already has this value
		if (node == _root)
			return _root->faceCount;
		else
		{
			// loop through the node's region of the grid and count the visible faces
			unsigned int xMax = node->x + node->size;
			unsigned int yMax = node->y + node->size;
			unsigned int zMax = node->z + node->size;
			
			for (unsigned int z = node->z; z < zMax; ++z)
			{
				for (unsigned int y = node->y; y < yMax; ++y)
				{
					for (unsigned int x = node->x; x < xMax; ++x)
					{
						unsigned long long index = x + y * _gridSize + z * _gridSize * _gridSize;
						unsigned char flags = (_grid[index] & 0xFF000000) >> 24;
						faceCount += _countBits(flags);
					}
				}
			}

			return faceCount;
		}
	}
	else
	{
		cout << "ERROR: _countVisibleFaces called with a NULL node! not cool." << endl;
		return 0;
	}
}

unsigned int Octree::_countBits(unsigned char b)
{
	// lazy way of mapping a byte to its length in 1's
	static unsigned int byteToBitCount[] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
											1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
											1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
											2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
											1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
											2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
											2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
											3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8};
	return byteToBitCount[b];
}

unsigned int Octree::_computeVisibleFaces()
{
	// the various offsets used to check adjacent voxels
	static int dx[] = {1,-1,0,0,0,0};
	static int dy[] = {0,0,1,-1,0,0};
	static int dz[] = {0,0,0,0,1,-1};

	unsigned int v, faceCount = 0;
	unsigned char faceFlags;

	// loop through the whole grid
	for (unsigned int z = 0; z < _gridSize; ++z)
	{
		for (unsigned int y = 0; y < _gridSize; ++y)
		{
			for (unsigned int x = 0; x < _gridSize; ++x)
			{
				unsigned long long index = x + y * _gridSize + z * _gridSize * _gridSize;
				v = _grid[index];

				// only check non-empty voxels
				if (v != 0)
				{
					faceFlags = 0;

					// loop through each face
					for (int face = 0; face < 6; ++face)
					{
						// compute adjacent voxel coord
						int tx = static_cast<int>(x) + dx[face];
						int ty = static_cast<int>(y) + dy[face];
						int tz = static_cast<int>(z) + dz[face];
						int intSize = static_cast<int>(_gridSize);

						// check if the current adjacent voxel coord is out of bounds, which will happen with edge voxels
						if (!((tx < 0) || (tx >= intSize) || (ty < 0) || (ty >= intSize) || (tz < 0) || (tz >= intSize)))
						{
							unsigned long long index2 = static_cast<unsigned int>(tx) + static_cast<unsigned int>(ty) * _gridSize + static_cast<unsigned int>(tz) * _gridSize * _gridSize;

							// if the adjacent voxel is empty, then the current face of the voxel is visible
							if (_grid[index2] == 0)
							{
								faceFlags |= (1 << face);
								faceCount++;
							}
						}
					}

					// if visible faces were found, save it in the upper byte of the grid voxel
					if (faceFlags != 0)
					{
						_grid[index] &= 0x00FFFFFF;
						_grid[index] |= (faceFlags << 24);
					}
				}
			}
		}
	}

	return faceCount;
}

void Octree::_buildLeafBuffer(Node* node)
{
	//cout << "building node..." << endl << "done!" << endl;
	_leafCount++;
}

void Octree::render(Shader* shader)
{
	unsigned int nodeCount = 0;
	shader->setUniform1i("useTexturing", 0);
	glVertexAttrib4f(1, 0.0f, 0.0f, 1.0f, 1.0f);

	// loop through all nodes and draw their VAO
	for (unsigned int i = 0; i < _nodes.size(); ++i)
	{
		// draw the 2 halves of the wireframe cube
		//glVertexAttrib4f(1, _nodes[i]->color.x, _nodes[i]->color.y, _nodes[i]->color.z, 1.0f);
		if (_nodes[i]->isLeaf || _nodes[i] == _root)
		{
			nodeCount++;
			glBindVertexArray(_nodes[i]->wireframeVAO);
			glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, 0);
			glDrawElements(GL_LINE_STRIP, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(unsigned short)));
		}
	}

	//cout << "node count " << nodeCount << endl;
}