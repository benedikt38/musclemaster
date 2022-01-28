#version 450 core

layout (location = 0) in vec4 Position;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 rotationMatrix;
uniform mat4 secondHalfRotationMatrix;
uniform vec4 sarcomereMidPoint;
uniform float myosinLength;
uniform int numLineSegments;
uniform float radius;
uniform float yOffset;
out vec4 passPos_G;
flat out float passRadius_G;

layout (std430, binding = 2) readonly buffer mRod_ssbo
{
	vec4 filamentOffset[];
};

layout (std430, binding = 7) readonly buffer LMMOffsetPositions_ssbo
{
	vec4 pieceOffset[];
};

void main(){
    //line segments per actin filament
    int filamentID = int(gl_InstanceID / numLineSegments);
    int linepieceID = gl_InstanceID % numLineSegments;
    passRadius_G = radius;

    if(linepieceID < numLineSegments / 2){
        passPos_G = rotationMatrix * vec4(((secondHalfRotationMatrix * Position) + filamentOffset[filamentID] + pieceOffset[linepieceID]).xyz,1.0f);
        gl_Position = projectionMatrix * viewMatrix * passPos_G;
    }
    else
    {
        passPos_G = rotationMatrix * vec4((Position + filamentOffset[filamentID] + pieceOffset[linepieceID]).xyz,1.0f);
        gl_Position = projectionMatrix * viewMatrix * passPos_G;
    }
}
