#include "Shader.h"

void Shader::create(const char* vsf, const char* fsf)
{
	unsigned int vs, fs, p;

	vs = _loadShader(vsf, GL_VERTEX_SHADER);
	fs = _loadShader(fsf, GL_FRAGMENT_SHADER);

	if (vs != 0 && fs != 0)
	{
		p = _createProgram(vs, fs);

		if (p != 0)
		{
			_vertexShaders.push_back(vs);
			_fragmentShaders.push_back(fs);
			_program = p;
		}

	}
}

unsigned int Shader::_loadShader(const char* fileName, GLenum shaderType)
{
	unsigned int shader;

	if (shaderType == GL_VERTEX_SHADER)
		shader = glCreateShader(GL_VERTEX_SHADER);
	else if (shaderType == GL_FRAGMENT_SHADER)
		shader = glCreateShader(GL_FRAGMENT_SHADER);
	else
	{
		printf("error loading shader: invalid shader type\n");
		return 0;
	}

	fstream fileIn;
	string source;
	char buf[200];
	char* infoLog;
	char* sourcePtr;
	int status, infoLogLength;

	fileIn.open(fileName, ios:: in);
	if (fileIn)
	{
		while (!fileIn.eof())
		{
			fileIn.getline(buf, 200);
			source = source + string(buf) + "\n";
		}

		sourcePtr = (char*)source.c_str();
		glShaderSource(shader, 1, (const GLchar**)&sourcePtr, NULL);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

			infoLog = new char[infoLogLength];
			glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);

			if (shaderType == GL_VERTEX_SHADER)
				printf("error compiling vertex shader:\n%s\n", infoLog);
			else if (shaderType == GL_FRAGMENT_SHADER)
				printf("error compiling fragment shader:\n%s\n", infoLog);

			delete[] infoLog;
			glDeleteShader(shader);
			return 0;
		}

		return shader;		
	}
	else
	{
		printf("error loading shader file: %s\n", fileName);
		return 0;
	}
}

unsigned int Shader::_createProgram(unsigned int vp, unsigned int fp)
{
	unsigned int program = glCreateProgram();
	glAttachShader(program, vp);
	glAttachShader(program, fp);
	glLinkProgram(program);

	int status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		int infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* infoLog = new char[infoLogLength];
		glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
		
		printf("error linking shader program:\n%s\n", infoLog);
		delete[] infoLog;
		glDeleteProgram(program);
		return 0;
	}
	else
		return program;
}

int Shader::getUniform(const char* var)
{
	return glGetUniformLocation(_program, var);
}

unsigned int Shader::getProgram()
{
	return _program;
}

void Shader::setUniform1f(const char* var, float x)
{
	if (var != NULL)
	{
		unordered_map<string,int>::iterator i = _uniforms.find(string(var));
		if (i != _uniforms.end())
			glUniform1f(i->second, x);
		else
		{
			unsigned int loc = glGetUniformLocation(_program, var);
			if (loc != -1)
			{
				_uniforms[string(var)] = loc;
				glUniform1f(loc, x);
			}
		}
	}
}

void Shader::setUniform1f(unsigned int var, float x)
{
	//
}
		
void Shader::setUniform3f(const char* var, float x, float y, float z)
{
	if (var != NULL)
	{
		unordered_map<string,int>::iterator i = _uniforms.find(string(var));
		if (i != _uniforms.end())
			glUniform3f(i->second, x, y, z);
		else
		{
			unsigned int loc = glGetUniformLocation(_program, var);
			if (loc != -1)
			{
				_uniforms[string(var)] = loc;
				glUniform3f(loc, x, y, z);
			}
		}
	}
}

void Shader::setUniform3f(unsigned int var, float x, float y, float z)
{
	//
}

void Shader::setUniform1i(const char* var, int x)
{
	if (var != NULL)
	{
		unordered_map<string, int>::iterator i = _uniforms.find(string(var));
		if (i != _uniforms.end())
			glUniform1i(i->second, x);
		else
		{
			unsigned int loc = glGetUniformLocation(_program, var);
			if (loc != -1)
			{
				_uniforms[string(var)] = loc;
				glUniform1i(loc, x);
			}
		}
	}
}

void Shader::setUniform1i(unsigned int var, int x)
{
	//
}
		
void Shader::setMatrix4f(const char* var, float* m)
{
	if (var != NULL)
	{
		unordered_map<string, int>::iterator i = _uniforms.find(string(var));
		if (i != _uniforms.end())
			glUniformMatrix4fv(i->second, 1, GL_FALSE, m);
		else
		{
			unsigned int loc = glGetUniformLocation(_program, var);
			if (loc != -1)
			{
				_uniforms[string(var)] = loc;
				glUniformMatrix4fv(loc, 1, GL_FALSE, m);
			}
		}
	}
}

void Shader::setMatrix4f(unsigned int var, float* m)
{
	//
}

void Shader::setMatrix4f(const char* var, const Matrix4& m)
{
	if (var != NULL)
	{
		unordered_map<string, int>::iterator i = _uniforms.find(string(var));
		if (i != _uniforms.end())
			glUniformMatrix4fv(i->second, 1, GL_FALSE, m.m);
		else
		{
			unsigned int loc = glGetUniformLocation(_program, var);
			if (loc != -1)
			{
				_uniforms[string(var)] = loc;
				glGetError();
				glUniformMatrix4fv(loc, 1, GL_FALSE, m.m);
				if (glGetError())
					cout << "uniform fail ";
			}
		}
	}
}

void Shader::setMatrix4f(unsigned int var, const Matrix4& m)
{
	glUniformMatrix4fv(var, 1, GL_FALSE, m.m);
}


void Shader::use()
{
	glUseProgram(_program);
}

void Shader::destroy()
{
	glDeleteProgram(_program);

	for (std::list<unsigned int>::iterator i = _vertexShaders.begin(); i != _vertexShaders.end(); i++)
		glDeleteShader(*i);

	for (std::list<unsigned int>::iterator i = _fragmentShaders.begin(); i != _fragmentShaders.end(); i++)
		glDeleteShader(*i);

	_program = 0;
	_vertexShaders.clear();
	_fragmentShaders.clear();
}
