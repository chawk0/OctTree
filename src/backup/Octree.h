#pragma once

#include "ChawkFortress.h"

#include "Shader.h"

class Octree
{
	public:
		Octree();
		~Octree();

		void build(unsigned int* grid, unsigned int gridSize, unsigned int threshold);
		void render(Shader* shader);

	private:
		struct Buffer
		{
			float* data;
			unsigned int index, size, offset;
			int componentCount;
			int elementCount;
		};

		// represents the visible faces of each voxel, and is used to build render buffer
		struct FaceData
		{
			unsigned int x, y, z;
			unsigned int faceFlags;
		};

		// The Node
		struct Node
		{
			// basic node pointers
			Node* parent;
			Node* child[8];
			bool isLeaf;

			// size of the node.  spans from (x,y,z) to (x+size,y+size,z+size)
			unsigned int x, y, z;
			unsigned int size;

			// render variables to draw a wireframe box around the node
			unsigned int wireframeVAO;
			unsigned int wireframeVBO;
			unsigned int wireframeIBO;
			Vector3 color;

			// and the actual world data in the leaf
			unsigned int leafVAO;
			unsigned int leafVBO;
			Buffer leafBuffer;
			unsigned int faceCount;
		};

		// computes visibility flags for the entire grid
		unsigned int _computeVisibleFaces();
		// used in recursion to find the face count in a node's region
		unsigned int _countVisibleFaces(Node* node);
		unsigned int _countBits(unsigned char b);

		// functions to assemble and build the render buffers
		void _buildWireframeBuffer(Node* node);
		void _buildLeafBuffer(Node* node);
		
		// the actual recursion function to build the tree.  the return value indicates whether
		// the given node had any visible faces or not.
		bool _buildRecurse(Node* node);
		
		Node* _root;
		unsigned int* _grid;
		unsigned int _gridSize, _threshold;
		unsigned int _leafCount;
		vector<Node*> _nodes;
};

