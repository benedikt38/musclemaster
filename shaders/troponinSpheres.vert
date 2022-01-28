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

layout (std430, binding = 6) readonly buffer troponin_ssbo
{
	vec4 particleOffset[];
};


void main() 
{
	int id = gl_InstanceID % numParticles;
	int filamentID = gl_InstanceID / numParticles;
	vec4 pos = Position;
	//scale point radius
	pos = scaleWidthMatrix * pos;
	pos = pos + particleOffset[id];
	pos = pos + filamentOffset[filamentID];
	pos = rotationMatrix * pos;
	pos = vec4(pos.xyz, 1.0f);
	pos = viewMatrix * pos;
	float pointScale = viewportY * 0.7f * projectionMatrix[1][1];
	passPosition = pos.xyz;
	passNormal = Normal;
	passProjMat = projectionMatrix;
	gl_Position = projectionMatrix * pos; 
	gl_PointSize = (basePointSize * pointScale) / gl_Position.w;
	passPointSize = basePointSize;
}

