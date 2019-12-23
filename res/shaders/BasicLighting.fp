#version 330

const int MAX_POINT_LIGHTS = 8;
const int MAX_SPOT_LIGHTS = 8;

struct BaseLight
{
	vec3 color;
	float ambient;
	float diffuse;
};

struct DirectionalLight
{
	BaseLight base;
	vec3 direction;
};

struct Attenuation
{
	float a;
	float b;
	float c;
};

struct PointLight
{
	BaseLight base;
	vec3 pos;
	Attenuation attenuation;
};

struct SpotLight
{
	BaseLight base;
	vec3 pos;
	vec3 direction;
	float cutoff;
	Attenuation attenuation;
};

uniform sampler2D baseMap;
uniform sampler2D shadowMap;

uniform bool useLighting;
uniform bool useTexturing;
uniform bool useHeightFactor;
uniform float worldHeight;

uniform int pointLightCount;
uniform int spotLightCount;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

in vec3 pos0;
in vec3 color0;
in vec2 uv00;
in vec4 lightSpacePos0;
in vec3 normal0;

out vec4 fragOut;

float calcShadowFactor(vec4 lightSpacePos)
{
	vec3 ndc = lightSpacePos.xyz / lightSpacePos.w;
	vec2 uv;
	uv.x = 0.5 * ndc.x + 0.5;
	uv.y = 0.5 * ndc.y + 0.5;
	float z = 0.5 * ndc.z + 0.5;
	float depth = texture(shadowMap, uv).x;

	if (depth < (z - 0.001))
		return 0.5f;
	else
		return 1.0;
}

vec4 calcDirectionalLight(DirectionalLight light, vec3 normal)
{
	vec4 ambientColor = vec4(light.base.color, 1.0) * light.base.ambient;
	vec4 diffuseColor = vec4(light.base.color, 1.0) * light.base.diffuse * clamp(dot(normal, light.direction), 0.0, 1.0);

	return ambientColor + diffuseColor;
}

vec4 calcPointLight(PointLight light, vec3 normal)
{
	vec4 ambientColor = vec4(light.base.color, 1.0) * light.base.ambient;
	float d = length(light.pos - pos0);
	float attenuationFactor = light.attenuation.a + light.attenuation.b * d + light.attenuation.c * d * d;
	vec4 diffuseColor = vec4(light.base.color, 1.0) * light.base.diffuse * clamp(dot(normal, normalize(light.pos - pos0)), 0.0, 1.0) / attenuationFactor;

	return ambientColor + diffuseColor;
}

vec4 calcSpotLight(SpotLight light, vec3 normal, vec4 lightSpacePos)
{
	vec4 ambientColor = vec4(light.base.color, 1.0) * light.base.ambient;
	vec4 diffuseColor;

	float visibility = dot(normal, normalize(light.pos - pos0));
	if (visibility > 0.0)
	{
		float coneFactor = dot(normalize(pos0 - light.pos), light.direction);
		if (coneFactor >= light.cutoff)
		{
			float spotFactor = (coneFactor - light.cutoff) / (1.0 - light.cutoff);
			float shadowFactor = calcShadowFactor(lightSpacePos);
			float d = length(light.pos - pos0);
			float attenuationFactor = light.attenuation.a + light.attenuation.b * d + light.attenuation.c * d * d;
			diffuseColor = vec4(light.base.color, 1.0) * spotFactor * shadowFactor / attenuationFactor;
		}
		else
			diffuseColor = vec4(0.0);
	}
	else
		diffuseColor = vec4(0.0);
	
	return ambientColor + diffuseColor;	
}

void main()
{
	if (useLighting)
	{
		vec4 totalLight = calcDirectionalLight(directionalLight, normal0);

		for (int i = 0; i < pointLightCount; ++i)
			totalLight += calcPointLight(pointLights[i], normal0);

		for (int i = 0; i < spotLightCount; ++i)
			totalLight += calcSpotLight(spotLights[i], normal0, lightSpacePos0);

		if (useTexturing)
		{
			if (useHeightFactor)
			{
				float heightFactor = clamp(pos0.y / worldHeight, 0.0f, 1.0f) * 1.0f + 0.0f;
				fragOut = totalLight * vec4(color0, 1.0) * vec4(texture(baseMap, uv00).rgb, 1.0) * heightFactor;
			}
			else
				fragOut = totalLight * vec4(color0, 1.0) * vec4(texture(baseMap, uv00).rgb, 1.0);
		}
		else
			fragOut = totalLight * vec4(color0, 1.0);
	}
	else
	{
		if (useTexturing)
		{
			if (useHeightFactor)
			{
				float heightFactor = clamp(pos0.y / worldHeight, 0.0f, 1.0f) * 1.0f + 0.0f;
				fragOut = vec4(color0, 1.0) * vec4(texture(baseMap, uv00).rgb, 1.0) * heightFactor;
			}
			else
				fragOut = vec4(color0, 1.0) * vec4(texture(baseMap, uv00).rgb, 1.0);
		}
		else
			fragOut = vec4(color0, 1.0);
	}
}