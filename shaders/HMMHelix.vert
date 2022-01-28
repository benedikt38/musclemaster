#version 450 core

layout (location = 0) in vec4 Position;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 rotationMatrix;
uniform mat4 secondHalfRotationMatrix;
uniform int numLineSegments;
uniform float radius;
out vec4 passPos_G;
flat out float passRadius_G;

layout (std430, binding = 2) readonly buffer mRod_ssbo
{
	vec4 filamentOffset[];
};

layout (std430, binding = 8) readonly buffer HMMOffsetPositions_ssbo
{
	vec4 pieceOffset[];
};

layout (std430, binding = 9) readonly buffer HMMyRotation_ssbo
{
	mat4 yRotation[];
};

layout (std430, binding = 10) readonly buffer HMMzRotation_ssbo
{
	mat4 zRotation[];
};

layout (std430, binding = 11) readonly buffer HMMyRotation2_ssbo
{
	mat4 yRotation2[];
};


void main(){
    //line segments per actin filament
    int filamentID = int(gl_InstanceID / numLineSegments);
    int linepieceID = gl_InstanceID % numLineSegments;
    passRadius_G = radius;
    if(linepieceID < numLineSegments / 2){
        passPos_G = rotationMatrix * vec4((yRotation[linepieceID] * yRotation2[linepieceID] * (secondHalfRotationMatrix * zRotation[linepieceID] * Position) + filamentOffset[filamentID] + pieceOffset[linepieceID]).xyz,1.0f);
        gl_Position = projectionMatrix * viewMatrix * passPos_G;
    }
    else
    {
        passPos_G = rotationMatrix * vec4((yRotation[linepieceID - numLineSegments / 2] * yRotation2[linepieceID - numLineSegments / 2] * (zRotation[linepieceID - numLineSegments / 2] * Position) + filamentOffset[filamentID] + pieceOffset[linepieceID]).xyz,1.0f);
        gl_Position = projectionMatrix * viewMatrix * passPos_G;
    }
}
