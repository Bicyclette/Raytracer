#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 3) in mat4 instanceModel;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform bool computeWorldPos;
uniform bool instancing;

void main()
{
	if(instancing)
	{
		if(computeWorldPos)
			gl_Position = instanceModel * vec4(aPos, 1.0f);
		else
			gl_Position = proj * view * instanceModel * vec4(aPos, 1.0f);
	}
	else
	{
		if(computeWorldPos)
			gl_Position = model * vec4(aPos, 1.0f);
		else
			gl_Position = proj * view * model * vec4(aPos, 1.0f);
	}

}
