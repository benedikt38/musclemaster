#version 450 core

layout (location = 0) in vec4 Position;
layout (location = 1) in vec3 Normal;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 rotationMatrix;
uniform mat4 translationMatrix;
uniform mat4 scaleWidthMatrix;
uniform mat4 scaleHeightMatrix;
uniform float sarcomereLength;
uniform float myosinLength;
out vec3 passPosition;
out vec3 passNormal;

layout (std430, binding = 2) readonly buffer mRod_ssbo
{
	vec4 offset[];
};

void main() 
{
	int id = gl_InstanceID;
	vec4 pos = viewMatrix * vec4((rotationMatrix * ((scaleHeightMatrix * scaleWidthMatrix * Position) + offset[id] + vec4(0.0f, (-sarcomereLength - myosinLength) / 2.0f + sarcomereLength / 2.0f, 0.0f, 0.0f))).xyz,1.0f);
	mat3 normalMatrix = mat3(transpose(inverse(viewMatrix * rotationMatrix)));
	passPosition = pos.xyz;
	passNormal = normalize(normalMatrix * Normal);
	gl_Position = projectionMatrix * pos; 
}

