#include "Sketch.h"

int Sketch::_state = READY;
unordered_map<string, Sketch::Instance> Sketch::_instances;
unordered_map<string, Sketch::Instance>::iterator Sketch::_current;

/*
void Sketch::init()
{
	// unneeded!
	//_state = READY;
}
*/


/*
void Sketch::begin(GLenum mode, char* name)
{
	begin(mode, name, false);
}*/

void Sketch::begin(GLenum mode, char* name)//, bool renderImmediately)
{
	Instance newInstance;
	unordered_map<string, Instance>::iterator i;
	
	// ignore begin calls while building/skipping
	if (_state == READY)
	{
		i = _instances.find(name);
		// either add new instance
		if (i == _instances.end())
		{
			newInstance._name = string(name);
			newInstance._renderMode = mode;
			newInstance._elementCount = 0;
			newInstance._currentColor = Vector3(1.0f, 1.0f, 1.0f); // white default color
			newInstance._vao = 0;
			newInstance._vbo = 0;

			// insert new instance and start building
			_instances.insert(unordered_map<string, Instance>::value_type(string(name), newInstance));
			_state = BUILDING;
			_current = _instances.find(string(name));
		}
		// or render the existing sketch and ignore all the vertex/color calls until end()
		else
		{	
			glBindVertexArray(i->second._vao);
			glDrawArrays(i->second._renderMode, 0, i->second._elementCount);
			
			//e = glGetError(); if(e != 0) printf("gl error: %d\n", e);

			_state = SKIPPING;
		}
	}
}

void Sketch::vertex3f(float x, float y, float z)
{
	if (_state == BUILDING)
	{
		_current->second._vertexData.push_back(x);
		_current->second._vertexData.push_back(y);
		_current->second._vertexData.push_back(z);

		_current->second._colorData.push_back(_current->second._currentColor.x);
		_current->second._colorData.push_back(_current->second._currentColor.y);
		_current->second._colorData.push_back(_current->second._currentColor.z);

		_current->second._elementCount++;
	}
}

void Sketch::vertex3fv(float* v)
{
	if (_state == BUILDING)
	{
		_current->second._vertexData.push_back(v[0]);
		_current->second._vertexData.push_back(v[1]);
		_current->second._vertexData.push_back(v[2]);

		_current->second._colorData.push_back(_current->second._currentColor.x);
		_current->second._colorData.push_back(_current->second._currentColor.y);
		_current->second._colorData.push_back(_current->second._currentColor.z);

		_current->second._elementCount++;
	}
}

void Sketch::vertex3(Vector3& v)
{
	if (_state == BUILDING)
	{
		_current->second._vertexData.push_back(v.x);
		_current->second._vertexData.push_back(v.y);
		_current->second._vertexData.push_back(v.z);

		_current->second._colorData.push_back(_current->second._currentColor.x);
		_current->second._colorData.push_back(_current->second._currentColor.y);
		_current->second._colorData.push_back(_current->second._currentColor.z);

		_current->second._elementCount++;
	}
}

void Sketch::color3f(float r, float g, float b)
{
	if (_state == BUILDING)
		_current->second._currentColor = Vector3(r, g, b);
}

void Sketch::color3fv(float* c)
{
	if (_state == BUILDING)
	_current->second._currentColor = Vector3(c);
}

void Sketch::color3(Vector3& c)
{
	if (_state == BUILDING)
	_current->second._currentColor = c;
}

void Sketch::end()
{
	Instance& i = _current->second;

	// if we were building, then wrap things up and gen buffers
	if (_state == BUILDING)
	{
		// make sure some data was given
		if (i._vertexData.size() > 0)
		{
			printf("building sketch %s...\n", i._name.c_str());

			glGenVertexArrays(1, &i._vao);
			glBindVertexArray(i._vao);
			
			glGenBuffers(1, &i._vbo);
			glBindBuffer(GL_ARRAY_BUFFER, i._vbo);
			
			// add the color buffer at the end of the vertex buffers
			i._vertexData.insert(i._vertexData.end(), i._colorData.begin(), i._colorData.end());

			// upload data
			//static float v[] = {-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};
			//glBufferData(GL_ARRAY_BUFFER, sizeof(v) , v, GL_STATIC_DRAW);
			glBufferData(GL_ARRAY_BUFFER, i._vertexData.size() * sizeof(float) , &i._vertexData[0], GL_STATIC_DRAW);
			
			// setup attribute pointers
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0); 
			glEnableVertexAttribArray(1);
			int colorBufferOffset = (int)(i._vertexData.size() - i._colorData.size()) * sizeof(float);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (GLvoid*)(colorBufferOffset));
			
			glBindVertexArray(0);
			glDrawArrays(i._renderMode, 0, i._elementCount);
		}
		else
			// else don't save this instance after all
			_instances.erase(_instances.find(string(i._name)));

		// ready either way
		_state = READY;
	}
	else if (_state == SKIPPING)
	{
		// clear the current VAO
		glBindVertexArray(0);
		// stop ignoring vertex/color calls
		_state = READY;
	}
}
