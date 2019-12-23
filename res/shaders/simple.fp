#version 400

in vec3 color0;
smooth in vec4 pos0;

out vec4 fragOut;

void main()
{
    //fragOut = vec4(color0, 1.0);
	
	vec4 p = pos0;
	p.xyz /= p.w;
	/*
	if (p.z < -1.0)
		fragOut = vec4(1.0, 0.0, 0.0, 1.0);
	else if (p.z > 1.0)
		fragOut = vec4(0.0, 1.0, 0.0, 1.0);
	else
	{
		p = p / 2.0f + vec4(0.5, 0.5, 0.5, 0.0);
		fragOut = vec4(p.z, p.z, p.z, 1.0);
	}
	*/
	
	fragOut = vec4(p.z, p.z, p.z, 1.0);
	//fragOut = vec4(0.2, 0.2, 1.0, 1.0);
}