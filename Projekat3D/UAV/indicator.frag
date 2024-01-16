#version 330 core

out vec4 outCol;
uniform vec3 newCol;

void main()
{
	outCol = vec4(newCol, 1.0);
}