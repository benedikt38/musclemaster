#version 450 core

layout (location = 0) in vec4 Position;
layout (location = 1) in vec3 Normal;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 rotationMatrix;
uniform mat4 scaleWidthMatrix;
uniform int numParticles;
uniform int viewportY;
uniform float basePointSize;
out vec3 passPosition;
out vec3 passNormal;
out float passPointSize;
out mat4 passProjMat;

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
	int id = gl_InstanceID % numParticles;
	int filamentID = gl_InstanceID / numParticles;
	vec4 pos = viewMatrix * vec4((rotationMatrix * ((scaleWidthMatrix * Position) + filamentOffset[filamentID] + particleOffset[id])).xyz,1.0f);
	float pointScale = viewportY * 0.7f * projectionMatrix[1][1];
	passPosition = pos.xyz;
	passNormal = Normal;
	passProjMat = projectionMatrix;
	gl_Position = projectionMatrix * pos; 
	gl_PointSize = (basePointSize * pointScale) / gl_Position.w;
	passPointSize = basePointSize;
}

