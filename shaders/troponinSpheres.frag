#version 450 core

uniform vec3 diffColor;
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
	//vec3 diffColor = vec3(0.0f,0.0f,0.5f);
    vec3 sphereNormal;

	//calculate shading for points to look like spheres if gl_PointCoord is inside the sphere
	if(length(gl_PointCoord * 2.0f - 1.0f) <= 1.0f)
	{
		// calculate viewspace sphere normal
		sphereNormal.xy = gl_PointCoord * vec2(2.0f, -2.0f) + vec2(-1.0f, 1.0f);
		float r2 = dot(sphereNormal.xy, sphereNormal.xy);
   		sphereNormal.z = sqrt(1.0f - r2);
		sphereNormal = normalize(sphereNormal);
		// calculate depth on sphere
		vec4 viewSpacePos = vec4(passPosition + sphereNormal * passPointSize, 1.0f);   // position of this pixel on sphere in view space
		vec4 clipSpacePos = passProjMat * viewSpacePos;
		gl_FragDepth = (clipSpacePos.z / clipSpacePos.w) * 0.5f + 0.5f;
		// calculate grayscale color
		float diffuseShade = max(0.0f, dot(sphereNormal, normalize(lightDir)));

		// Specular term
		vec3 eye = normalize(-passPosition);
		vec3 reflection = normalize(reflect(-lightDir, sphereNormal));
		float cos_psi_n = pow(max(dot(reflection, eye), 0.0f), 15);

		//sum up colors
		frag_Color.rgb = baseColor;
		frag_Color.rgb += diffColor * vec3(diffuseShade, diffuseShade, diffuseShade) * lightColor;
		frag_Color.rgb += specColor * cos_psi_n * lightColor;
		frag_Color.a;
	}
	//discard pixel if gl_PointCoord is outside of the sphere
	else
	{
		discard;
	}
}