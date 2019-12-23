#include "Light.h"


void Light::setType(Type type)
{
	_type = type;
}

void Light::setPos(float x, float y, float z)
{
	_pos.x = x;
	_pos.y = y;
	_pos.z = z;
	if (_bound)
		glUniform3f(_uniforms[POS], _pos.x, _pos.y, _pos.z);	
}		

void Light::setPos(Vector3& pos)
{
	_pos = pos;
	if (_bound)
		glUniform3f(_uniforms[POS], _pos.x, _pos.y, _pos.z);	
}

void Light::setColor(float r, float g, float b)
{
	_color.x = r;
	_color.y = g;
	_color.z = b;
	if (_bound)
		glUniform3f(_uniforms[COLOR], _color.x, _color.y, _color.z);
}

void Light::setColor(Vector3& color)
{
	_color = color;
	if (_bound)
		glUniform3f(_uniforms[COLOR], _color.x, _color.y, _color.z);
}

void Light::setDirection(float x, float y, float z)
{
	_direction.x = x;
	_direction.y = y;
	_direction.z = z;
	if (_bound)
		glUniform3f(_uniforms[DIRECTION], _direction.x, _direction.y, _direction.z);
}

void Light::setDirection(Vector3& direction)
{
	_direction = direction;
	if (_bound)
		glUniform3f(_uniforms[DIRECTION], _direction.x, _direction.y, _direction.z);
}

void Light::lookAt(float x, float y, float z)
{
	_direction = Vector3(x, y, z) - _pos;
	_direction.normalize();
	if (_bound)
		glUniform3f(_uniforms[DIRECTION], _direction.x, _direction.y, _direction.z);
}

void Light::lookAt(Vector3& target)
{
	_direction = target - _pos;
	_direction.normalize();
	if (_bound)
		glUniform3f(_uniforms[DIRECTION], _direction.x, _direction.y, _direction.z);
}

void Light::setAmbient(float ambient)
{
	_ambient = ambient;
	if (_bound)
		glUniform1f(_uniforms[AMBIENT], _ambient);	
}

void Light::setDiffuse(float diffuse)
{
	_diffuse = diffuse;
	if (_bound)
		glUniform1f(_uniforms[DIFFUSE], _diffuse);
}

void Light::setCutoff(float cutoff)
{
	_cutoff = cutoff;
	if (_bound)
		glUniform1f(_uniforms[CUTOFF], _cutoff);
}

void Light::setAttenuation(float a, float b, float c)
{
	_a = a;
	_b = b;
	_c = c;

	if (_bound)
	{
		glUniform1f(_uniforms[A], _a);
		glUniform1f(_uniforms[B], _b);
		glUniform1f(_uniforms[C], _c);
	}
}

void Light::setProjection(int width, int height, float fovY, float zNear, float zFar)
{
	_projectionMatrix.makeProjection((float)width / (float)height, fovY, zNear, zFar);
}

void Light::setWorldMatrix(Matrix4& m)
{
	_worldMatrix = m;
}

void Light::setViewMatrix(Matrix4& m)
{
	_viewMatrix = m;
}

void Light::loadModel(const char* fileName)
{
	if (_model != NULL)
		delete _model;
	_model = new Mesh();

	_model->load("model", fileName);
	_model->setVertexAttribIndex("pos", 0);
	_model->build();
}

void Light::render()
{
	if (_model != NULL)
	{
		_model->render();
	}
}

void Light::createShadowMap(int width, int height)
{
	printf("creating shadowmap...");
	glGetError();
	if (_shadowMapFBO != 0)
	{
		glDeleteTextures(1, &_shadowMapTexture);
		glDeleteFramebuffers(1, &_shadowMapFBO);
	}

	glGenFramebuffers(1, &_shadowMapFBO);
	glGenTextures(1, &_shadowMapTexture);
	
	glBindTexture(GL_TEXTURE_2D, _shadowMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _shadowMapFBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadowMapTexture, 0);

	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("error creating framebuffer, status: 0x%x\n", status);

	_worldMatrix.makeIdentity();
	_computeViewMatrix();
	float fovY = 2.0f * acosf(_cutoff) / PI_OVER_180;
	_projectionMatrix.makeProjection(1.6f, fovY, 1.0f, 100.0f);

	printf("done. %d\n", glGetError());
}

void Light::_computeViewMatrix()
{
	Vector3 right, up, forward, tempUp(0.0f, 1.0f, 0.0f);

	forward = _direction;
	
	if (forward.isEqualEpsilon(tempUp))
		tempUp = Vector3(1.0f, 0.0f, 0.0f);

	right = forward.cross(tempUp);
	up = right.cross(forward);
	
	_viewMatrix.makeIdentity();
	_viewMatrix.setRight(right);
	_viewMatrix.setUp(up);
	_viewMatrix.setForward(-forward);
}

void Light::bindShadowMapForWriting()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _shadowMapFBO);
}

void Light::bindShadowMapForReading(int index)
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, _shadowMapTexture);
}

Light::Type Light::getType()
{
	return _type;
}

Vector3 Light::getPos()
{
	return _pos;
}

Vector3 Light::getColor()
{
	return _color;
}

Vector3 Light::getDirection()
{
	return _direction;
}

float Light::getAmbient()
{
	return _ambient;
}

float Light::getDiffuse()
{
	return _diffuse;
}

float Light::getCutoff()
{
	return _cutoff;
}

void Light::updateShader()
{
	Matrix4 t, v, wvp;

	// compute the full view transform
	t.makeTranslate(-_pos);
	v = _viewMatrix * t;
	// concatenate into 1 wvp matrix for the shader
	wvp = _projectionMatrix * v * _worldMatrix;
	glUniformMatrix4fv(_wvpMatrixLoc, 1, GL_FALSE, wvp.m);
}

void Light::bind(Shader* program, int index)
{
	if (program == NULL)
		return;

	printf("setting light parameters...\n");
	glGetError();

	_bound = true;
	_program = program;

	string prefix;
	char i = '0' + (char)index;

	// retrieve and store the various uniform locations so they can be updated
	switch (_type)
	{
		case DIRECTIONAL:
			prefix = "directionalLight";

			_uniforms[DIRECTION] = _program->getUniform(string(prefix + ".direction").c_str());
			glUniform3f(_uniforms[DIRECTION], _direction.x, _direction.y, _direction.z);
			break;

		case POINT:
			prefix = "pointLights[" + string(1, i) + "]";
	
			_uniforms[POS] = _program->getUniform(string(prefix + ".pos").c_str());
			_uniforms[A] = _program->getUniform(string(prefix + ".attenuation.a").c_str());
			_uniforms[B] = _program->getUniform(string(prefix + ".attenuation.b").c_str());
			_uniforms[C] = _program->getUniform(string(prefix + ".attenuation.c").c_str());
			glUniform3f(_uniforms[POS], _pos.x, _pos.y, _pos.z);
			glUniform1f(_uniforms[A], _a);
			glUniform1f(_uniforms[B], _b);
			glUniform1f(_uniforms[C], _c);
			break;

		case SPOT:
			prefix = "spotLights[" + string(1, i) + "]";
			
			_uniforms[POS] = _program->getUniform(string(prefix + ".pos").c_str());
			_uniforms[DIRECTION] = _program->getUniform(string(prefix + ".direction").c_str());
			_uniforms[CUTOFF] = _program->getUniform(string(prefix + ".cutoff").c_str());
			_uniforms[A] = _program->getUniform(string(prefix + ".attenuation.a").c_str());
			_uniforms[B] = _program->getUniform(string(prefix + ".attenuation.b").c_str());
			_uniforms[C] = _program->getUniform(string(prefix + ".attenuation.c").c_str());
			glUniform3f(_uniforms[POS], _pos.x, _pos.y, _pos.z);
			glUniform3f(_uniforms[DIRECTION], _direction.x, _direction.y, _direction.z);
			glUniform1f(_uniforms[CUTOFF], _cutoff);
			glUniform1f(_uniforms[A], _a);
			glUniform1f(_uniforms[B], _b);
			glUniform1f(_uniforms[C], _c);
			break;
	}

	// all lights use these uniforms
	_uniforms[COLOR] = _program->getUniform(string(prefix + ".base.color").c_str());
	_uniforms[AMBIENT] = _program->getUniform(string(prefix + ".base.ambient").c_str());
	_uniforms[DIFFUSE] = _program->getUniform(string(prefix + ".base.diffuse").c_str());
	glUniform3f(_uniforms[COLOR], _color.x, _color.y, _color.z);
	glUniform1f(_uniforms[AMBIENT], _ambient);
	glUniform1f(_uniforms[DIFFUSE], _diffuse);

	// get the wvp matrix loc
	_wvpMatrixLoc = _program->getUniform("lightWVPMatrix");

	printf("done. %d\n", glGetError());
}

void Light::unbind()
{
	_bound = false;
	_program = NULL;
	memset(_uniforms, 0, sizeof(int) * 9);
}

void Light::destroy()
{
	_type = NONE;
	_program = NULL;

	if (_model)
		delete _model;
	_model = NULL;

	if (_shadowMapFBO)
		glDeleteFramebuffers(1, &_shadowMapFBO);
	_shadowMapFBO = 0;
	if (_shadowMapTexture)
		glDeleteTextures(1, &_shadowMapTexture);
	_shadowMapTexture = 0;

	_bound = false;
	_pos = Vector3(0.0f, 0.0f, 0.0f);
	_color = Vector3(1.0f, 1.0f, 1.0f);
	_direction = Vector3(0.0f, 0.0f, 0.0f);
	_ambient = 0.0f; _diffuse = 1.0f;
	_cutoff = 0.0f;
	_a = 0.0f; _b = 0.0f; _c = 0.0f;
}
