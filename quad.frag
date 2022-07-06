#version 330 core
layout (location = 0) out vec4 o_color;

in vec4 v_color;

void main()
{
	o_color = v_color;
}
