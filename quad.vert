#version 330 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_color;

uniform mat4 u_projection;
uniform mat4 u_model;

out vec4 v_color;

void main()
{
	v_color = a_color;
	gl_Position = projection * u_model * vec4(a_position, 1);
}
