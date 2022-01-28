#version 450 core

uniform vec3 diffColor;
uniform mat4 viewMatrix;
in vec3 passPosition;
in vec3 passNormal;
in mat4 passProjMat;
in float passPointSize;
out vec4 frag_Color;

void main()  
{
	vec3 lightDir = vec3(0.577f, 0.577f, 0.577f);
	vec3 lightColor = vec3(1.0f,1.0f,1.0f);
	vec3 baseColor = vec3(0.1f,0.1f,0.1f);
	vec3 specColor = vec3(1.0f,1.0f,1.0f);
	//vec3 diffColor = vec3(0.0f,0.4f,0.0f);
    vec3 sphereNormal;

	float radiusX = 2.0f;
	float radiusY = 1.0f;

	vec2 skewedPointCoord = gl_PointCoord * vec2(radiusX, -radiusX) + vec2(-radiusY, radiusY);
	skewedPointCoord *= vec2(radiusX, radiusY);
	skewedPointCoord -= vec2(-radiusY, radiusY);
	skewedPointCoord /= vec2(radiusX, -radiusX);

	if(length(skewedPointCoord * 2.0f - 1.0f) <= 1.0f && length(skewedPointCoord * 2.0f - 1.0f) > 0.9f)
	{
		frag_Color.rgb = baseColor;
	}
	//discard pixel if gl_PointCoord is outside of the sphere
	else
	{
		discard;
	}
}