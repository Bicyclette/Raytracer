#version 460 core

layout (points) in;
layout (line_strip, max_vertices = 8) out;

in VS_OUT
{
	vec4 direction;
} gs_in[];

void main()
{
	vec4 direction = normalize(gs_in[0].direction);
	vec4 ortho = normalize(vec4(-direction.y, direction.x, 0.0f, 0.0f));
	
	// first ray
	// ---------
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + (2.5f * direction);
	EmitVertex();
	EndPrimitive();
	
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position - (2.5f * direction);
	EmitVertex();
	EndPrimitive();
	
	gl_Position = gl_in[0].gl_Position + (2.5f * direction);
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + (2.0f * direction) + 0.25f * ortho;
	EmitVertex();
	EndPrimitive();
	
	gl_Position = gl_in[0].gl_Position + (2.5f * direction);
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + (2.0f * direction) - 0.25f * ortho;
	EmitVertex();
	EndPrimitive();
}
