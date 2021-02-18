#version 460 core

layout (triangles) in;
layout (triangle_strip,max_vertices = 18) out;

uniform bool omniDepthRendering;
uniform mat4 omnilightViews[6];

out vec4 fragPos;

void main()
{
	if(omniDepthRendering)
	{
		for(int f = 0; f < 6; ++f)
		{
			gl_Layer = f;
			for(int v = 0; v < 3; ++v)
			{
				fragPos = gl_in[v].gl_Position;
				gl_Position = omnilightViews[f] * fragPos;
				EmitVertex();
			}
			EndPrimitive();
		}
	}
	else
	{
		gl_Position = gl_in[0].gl_Position;
		fragPos = gl_in[0].gl_Position;
		EmitVertex();
		
		gl_Position = gl_in[1].gl_Position;
		fragPos = gl_in[1].gl_Position;
		EmitVertex();
		
		gl_Position = gl_in[2].gl_Position;
		fragPos = gl_in[2].gl_Position;
		EmitVertex();
		
		EndPrimitive();
	}
}
