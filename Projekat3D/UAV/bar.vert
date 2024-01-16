#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec4 inCol;
out vec4 chCol;
uniform float batteryLevel;

void main()
{	

	if (inPos.x >= -1){
		gl_Position = vec4(-1.0 + inPos.x + (batteryLevel / 100), inPos.y, 0.0, 1.0);
		chCol = inCol;
	}
}