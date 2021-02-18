#version 460 core

float linearizeDepth(float depth)
{
	float near = 0.1f;
	float far = 100.0f;
	float ndc = depth * 2.0f - 1.0f;
	float z = (2.0f * near * far) / (far + near - ndc * (far - near));
	return z / far;
}

in vec4 fragPos;

uniform bool omnilightFragDepth;
uniform vec3 lightPosition;

void main()
{
	if(omnilightFragDepth)
	{
		float lightDistance = length(fragPos.xyz - lightPosition);
		lightDistance /= 100.0;
		gl_FragDepth = lightDistance;
	}
	else
		gl_FragDepth = gl_FragCoord.z;
}
