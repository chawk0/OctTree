#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Camera.h"
//#include "Engine.h"
//#include "Log.h"
#include "MatrixStack.h"
#include "Octree.h"
#include "Perlin.h"
#include "Mesh.h"
#include "Shader.h"
//#include "Sketch.h"
#include "Texture.h"
//#include "Timer.h"

class World
{
	public:
		World();
		~World();

		void setVoxelDefs(Array<Voxel>* voxelDefs);
		void create(const char* file);
		void render(Shader* shader, bool hideTerrain);
		void saveSegment(const char* name);
		void destroy();

		uint* getGrid();
		uint getGridSize();
		Array<Segment>* getSegments();
		Array<uint>* getSegmentMap();
		Array<uint>* getSegmentRenderList();

		void temp();

	private:
		void _loadSegments(const char* file);
		// finds all visible faces for all segments
		void _computeVisibleFaces();
		// finds individual visible faces for a voxel
		uint _computeVisibleFaces(uint x, uint y, uint z, Array<uint>* vtids);
		void _buildRenderBuffer();

		string _name;

		// world is represented as a 3D array of voxel ID's stored as 32bit unsigned ints
		uint* _grid;
		uint _width, _height, _depth;

		// pointer to voxel definitions array maintained by Engine
		Array<Voxel>* _voxelDefs;
		// internal array of segments that comprise the world
		Array<Segment> _segments;
		// render data for each segments
		Array<RenderData> _segmentRenderData;
		// holds the order in which segments are rendered.  needed to do transparency correctly
		Array<uint> _segmentRenderList;
		// sort-of hack that allows me to map a voxel id back to the segment it was generated from
		Array<uint> _segmentMap;
};

