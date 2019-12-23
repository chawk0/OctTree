#version 330

uniform sampler2D glyphMap;

in vec3 color0;
in vec2 uv0;

out vec4 fragOut;

void main()
{
	fragOut = vec4(color0, 1.0) * vec4(1.0, 1.0, 1.0, texture(glyphMap, uv0).r);
}