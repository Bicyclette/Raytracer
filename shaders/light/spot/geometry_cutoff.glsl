#version 460 core

layout (points) in;
layout (line_strip, max_vertices = 27) out;

in VS_OUT 
{
	vec4 direction;
	float cutOff;
} gs_in[];

void main()
{
	vec4 direction = normalize(gs_in[0].direction);
	vec4 ortho = normalize(vec4(-direction.y, direction.x, 0.0f, 0.0f));
	float cutOff = gs_in[0].cutOff;

	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + 5.0f * direction;
	EmitVertex();
	EndPrimitive();
	
	gl_Position = gl_in[0].gl_Position + 5.0f * direction;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + 4.5f * direction + 0.25f * ortho;
	EmitVertex();
	EndPrimitive();
	
	gl_Position = gl_in[0].gl_Position + 5.0f * direction;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + 4.5f * direction - 0.25f * ortho;
	EmitVertex();
	EndPrimitive();

	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + 3.0f * direction + 3.0f * (sin(cutOff) * ortho);
	EmitVertex();
	EndPrimitive();
	
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + 3.0f * direction - 3.0f * (sin(cutOff) * ortho);
	EmitVertex();
}
