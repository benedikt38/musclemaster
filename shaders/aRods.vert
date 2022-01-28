#version 450 core

layout (location = 0) in vec4 Position;
layout (location = 1) in vec3 Normal;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 rotationMatrix;
uniform mat4 translationMatrix;
uniform mat4 scaleWidthMatrix;
uniform mat4 scaleHeightMatrix;
uniform mat4 secondHalfRotationMatrix;
uniform float sarcomereLength;
out vec3 passPosition;
out vec3 passNormal;

layout (std430, binding = 3) readonly buffer aRod_ssbo
{
	vec4 filamentOffset[];
};

layout (std430, binding = 4) readonly buffer aSphere_ssbo
{
	vec4 particleOffset[];
};


void main() 
{
	int id = gl_InstanceID;
	mat3 normalMatrix;
	vec4 pos;
	if(gl_InstanceID >= filamentOffset.length()/2)
	{
		pos = viewMatrix * vec4((rotationMatrix * secondHalfRotationMatrix * ((scaleHeightMatrix * scaleWidthMatrix * Position) + filamentOffset[id] + vec4(0.0f, -sarcomereLength / 2.0f, 0.0f, 0.0f))).xyz, 1.0f);
		normalMatrix = mat3(transpose(inverse(viewMatrix * rotationMatrix * secondHalfRotationMatrix)));
	}
	else
	{
		pos = viewMatrix * vec4((rotationMatrix * ((scaleHeightMatrix * scaleWidthMatrix * Position) + filamentOffset[id] + vec4(0.0f, -sarcomereLength / 2.0f, 0.0f, 0.0f))).xyz,1.0f);
		normalMatrix = mat3(transpose(inverse(viewMatrix * rotationMatrix)));
	}
 	passNormal = normalize(normalMatrix * Normal);
	passPosition = pos.xyz;
	gl_Position = projectionMatrix * pos; 
}

