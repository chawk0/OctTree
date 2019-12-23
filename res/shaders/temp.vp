#version 330

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform vec3 lightPos;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 t;
layout (location = 4) in vec3 b;
layout (location = 5) in vec3 n;

out vec3 pos0;
out vec3 color0;
out vec2 uv0;

out float hidden;
out vec3 t0;
out vec3 b0;
out vec3 n0;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(pos, 1.0);
	
	// feed the world space position to the fragment shader so we have it per-pixel
	pos0 = (modelMatrix * vec4(pos, 1.0)).xyz;	
	color0 = color;
	uv0 = uv;

	// the TBN matrix vectors are defined in object space, so transform them into world space
	// but ignore translation
	t0 = (modelMatrix * vec4(t, 0.0)).xyz;
	b0 = (modelMatrix * vec4(b, 0.0)).xyz;
	n0 = (modelMatrix * vec4(n, 0.0)).xyz;
	//t0 = t;
	//b0 = b;
	//n0 = n;

	vec3 lightDir = lightPos - pos0;
	hidden = clamp(dot(normalize(lightDir), n0), 0.0, 1.0);
}
