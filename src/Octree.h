#pragma once

#include "ChawkFortress.h"

#include "Shader.h"
#include "Texture.h"
#include "AABox.h"
#include "Frustum.h"

class Octree
{
//private:
public:
	// forwarddeclrblegarg
	struct Node;

public:
	Octree();
	~Octree();

	// for now, you pass all the relevant data structures from World to Octree via arguments to build().  blarg.
	void build(uint* grid, uint gridSize, Array<Segment>* segments, Array<uint>* segmentMap, Array<Voxel>* voxelDefs, Array<uint>* segmentRenderList, uint threshold);
	// pass a shader and the projection-view combo matrix
	void render(Shader* shader, const Matrix4& projViewMatrix);
	void render2(Shader* shader, const Matrix4& projViewMatrix);
	void render3(Shader* shader, const Matrix4& projViewMatrix);
	void render4(Shader* shader, Frustum* frustum);
	void renderWireframes(Shader* shader, const Matrix4& projViewMatrix, Frustum* frustum);
	void renderNodeTest(Shader* shader, const Matrix4& projViewMatrix, Frustum* frustum, const Vector3& testPoint, uint maxDepth);
	void renderNodeTest2(Shader* shader, const Matrix4& projViewMatrix, Frustum* frustum, Octree::Node* node, Octree::Node* highlight);

//private:
	// The Node
	struct Node
	{
		// basic node stuff
		Node* parent;
		Node* child[8];
		bool isLeaf;
		bool isEmpty;
		// the root of the tree is depth 0, increasing in depth as it subdivides
		uint depth;

		// points to adjacent nodes at an equal or lower level. +X, -X, +Y, -Y, +Z, -Z order.
		// NOTE: this is only used for leaf nodes.  since empty space is still bounded by
		// a leaf (with isEmpty == true), the entire octree volume is thus contained by leaves,
		// and so only neighbor info is stored for leaves.  it is possible, however, to have
		// a neighboring node be a non-leaf node.
		Node* neighbors[6];

		// size of the node.  spans from [x,y,z] to (x+size,y+size,z+size)
		uint x, y, z;
		uint size;
		// axis-aligned bounding box for frustum culling
		AABox box;

		// the elementCount and offset are used as parameters to glDrawArrays to specify
		// which portion of the VBO to draw
		uint visibleFaceCount;
		Array<uint> elementCounts;
		Array<uint> offsets;

		// color to use when rendering this node's wireframe bounding box
		Vector3 color;
		// the precomputed world transform for the box
		Matrix4 wireframeWorldMatrix;
	};

	// ========== PRE BUILD ==========
	// wireframe bounding boxes are drawn from 1 buffer, just translated/scaled per node/draw call
	void _buildWireframeBuffer();
		
	// ========== IN BUILD ==========
	// the actual recursion function to build the tree
	void _buildRecurse(Node* node);

	// used in recursion to find the face count in a node's region
	uint _countVisibleFaces(Node* node);
	// the upper byte of a grid value is the visible face bitmask, so need this.
	uint _countBits(unsigned char b);

	// build geometry for the leaf node's portion of the world grid.
	void _buildLeafBuffer(Node* node);

	// ========== POST BUILD ==========
	// computes adjacent neighbor info for each leaf node
	void _computeAdjacencyInfo();
	// builds the GL buffers for rendering
	void _buildRenderData();

	// locates the node that contains the given point and is at depth no greater than the provided depth value
	Node* _findNode(uint x, uint y, uint z, uint depth);
	Node* _findNodeRecurse(Node* node, uint x, uint y, uint z, uint depth);

	void _render4Recurse(Node* node, uint segmentIdx, Frustum* frustum);

	// the root of the tree
	Node* _root;
	// provides sequential access to all nodes, in depth-first order
	vector<Node*> _nodes;
	uint _threshold;
	uint _leafCount;
	uint _treeDepth;
	uint _emptyLeafCount;

	// holds all the pointers and relevant info about the World upon which the Octree is built
	uint* _grid;
	uint _gridSize;
	Array<Segment>* _segments;
	Array<uint>* _segmentMap;
	Array<Voxel>* _voxelDefs;
	// holds the render order of segments so blending renders correctly
	Array<uint>* _segmentRenderList;

	// relevant structures for the wireframe and leaf buffers
	RenderData _wireframeRenderData;
	Array<RenderData> _segmentRenderData;
	// temporary float buffers that accumulate geometry as the tree builds
	Array<float*> _posPtrs;
	Array<float*> _uvPtrs;

	// render statistics
	uint _lastFrameLeafRenderCount;
};

