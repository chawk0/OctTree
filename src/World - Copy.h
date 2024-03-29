#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Camera.h"
//#include "Engine.h"
//#include "Log.h"
#include "MatrixStack.h"
#include "Perlin.h"
#include "SceneObject.h"
#include "Shader.h"
//#include "Sketch.h"
#include "Texture.h"
//#include "Timer.h"

class World
{
	public:
		World();
		~World();

		void create(int width, int height, int depth);
		void render(Camera& camera, Shader& program);

	private:
		// holds basic information about each type of voxel (soil, water, etc)
		struct Voxel
		{
			int id;
			string name;
			Texture* texture;
		};

		// used for each VBO generated by glGenBuffers
		struct Buffer
		{
			float* data;
			unsigned int index, len, offset;
			int componentCount;
			int elementCount;
		};

		// represents a portion of the world that is rendered as one unit (terrain layer, water layer, etc)
		struct Segment
		{
			string name;
			int faceCount;
			// rendering handles
			unsigned int vao, vbo;
			// keep client-side copy of vertex data streams
			//unordered_map<string, Buffer> buffers;
			Buffer buffer;
		};
		
		// represents the visible faces of each voxel, and is used to build render buffer
		struct FaceData
		{
			int x, y, z;
			unsigned int faceFlags;
		};

		void _loadVoxelDefs();
		void _generateLandscape();
		void _buildRenderBuffer();
		int _computeVisibleFaces(int x, int y, int z, FaceData* fd, bool checkWater);

		// world is represented as a 3D array of voxel ID's stored as 32bit signed ints
		int* _grid;
		int _width, _height, _depth;
		// holds the list of voxel definitions and world segments
		vector<Voxel> _voxelDefs;
		vector<Segment> _segments;

		SceneObject _object;
};

