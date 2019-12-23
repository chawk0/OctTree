#version 330

//uniform mat4 modelMatrix;
uniform vec3 staticColor;
uniform vec3 lightPos;

uniform sampler2D baseMap;
uniform sampler2D normalMap;

in vec3 pos0;
in vec3 color0;
in vec2 uv0;
in vec3 t0;
in vec3 b0;
in vec3 n0;
in float hidden;

out vec4 fragOut;

void main()
{
	//fragOut = texture(baseMap, uv0) + vec4(color0, 1.0) + vec4(staticColor, 1.0);
	//fragOut = texture(normalMap, uv0) + vec4(color0, 1.0) + vec4(staticColor, 1.0);
	
	//fragOut = vec4(color0, 1.0);
	//fragOut = vec4(0.2f, 0.5f, 1.0f, 1.0f);

	
	vec3 c, n, lightDir;
	mat3 tbn;

	c = texture(normalMap, uv0).xyz;
	c = 2.0 * c - vec3(1.0, 1.0, 1.0);
	tbn = mat3(t0, b0, n0);
	n = tbn * c;
	//lightDir = (transpose(modelMatrix) * vec4(lightPos - pos0, 0.0)).xyz;
	//lightDir = transpose(tbn) * lightDir;
	lightDir = lightPos - pos0;
		
	// just add 'em all together!  screw if-branching
	fragOut = vec4(color0, 1.0) + vec4(staticColor, 1.0) + texture(baseMap, uv0) * clamp(dot(normalize(lightDir), n), 0.0, 1.0) * hidden;
	//fragOut = vec4(vec3(1.0, 1.0, 1.0) * clamp(dot(normalize(lightDir), n), 0.0, 1.0), 1.0);
	//fragOut = vec4(clamp(pos0, 0.0, 1.0), 1.0);
}