#version 450 core

layout (location = 0) in vec4 Position;
layout (location = 1) in vec3 Normal;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 rotationMatrix;
uniform mat4 sarcomereRadius;
uniform float sarcomereLength;
out vec3 passPosition;
out vec3 passNormal;

layout (std430, binding = 1) readonly buffer zDisc_ssbo
{
	vec4 offset[];
};

void main() 
{
	int id = gl_InstanceID;
	vec4 pos;
	if(gl_InstanceID == 1)
	{
		pos = viewMatrix * vec4((rotationMatrix * ((sarcomereRadius * Position) + vec4(0.0f, sarcomereLength / 2.0f, 0.0f, 0.0f) + offset[id])).xyz,1.0f);
	}
	else
	{
		pos = viewMatrix * vec4((rotationMatrix * ((sarcomereRadius * Position) + vec4(0.0f, -sarcomereLength / 2.0f, 0.0f, 0.0f) + offset[id])).xyz,1.0f);
	}
	passPosition = pos.xyz;
	passNormal = Normal;
	gl_Position = projectionMatrix * pos; 
}

