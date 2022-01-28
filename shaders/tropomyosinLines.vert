#version 450 core

layout (location = 0) in vec4 Position;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 rotationMatrix;
uniform mat4 translationMatrix;
uniform mat4 secondHalfRotationMatrix;
uniform float sarcomereLength;
uniform int numLineSegments;
uniform float radius;
uniform float pointDist;
out vec4 passPos_G;
flat out float passRadius_G;

layout (std430, binding = 3) readonly buffer aRod_ssbo
{
	vec4 filamentOffset[];
};

layout (std430, binding = 5) readonly buffer lineRotation_ssbo
{
	mat4 lineRotations[];
};

void main(){
    //line segments per actin filament
    int filamentID = int(gl_InstanceID / numLineSegments);
    int linepieceID = gl_InstanceID % numLineSegments;
    passRadius_G = radius;

    if(linepieceID < numLineSegments / 2){
        passPos_G = rotationMatrix * vec4(((lineRotations[linepieceID] * Position) + filamentOffset[filamentID] + vec4(0.0f, 7.0f * pointDist * linepieceID, 0.0f, 0.0f)).xyz,1.0f);
        gl_Position = projectionMatrix * viewMatrix * passPos_G;
    }
    else
    {
        passPos_G = rotationMatrix * vec4(((lineRotations[linepieceID] * secondHalfRotationMatrix * Position) + filamentOffset[filamentID] + vec4(0.0f, - (7.0f * pointDist * (linepieceID - int(numLineSegments) / 2)), 0.0f, 0.0f)).xyz,1.0f);
        gl_Position = projectionMatrix * viewMatrix * passPos_G;
    }
}