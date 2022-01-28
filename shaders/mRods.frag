#version 450 core

uniform vec3 diffColor;
in vec3 passPosition;
in vec3 passNormal;
out vec4 frag_Color;

void main()  
{       
	vec3 lightDir = vec3(0.577f, 0.577f, 0.577f);
	vec3 lightColor = vec3(1.0f,1.0f,1.0f);
	vec3 baseColor = vec3(0.1f,0.1f,0.1f);
	vec3 specColor = vec3(1.0f,1.0f,1.0f);

	// Diffuse term
	lightDir = normalize(lightDir);
	float cos_phi = max(dot(passNormal, lightDir), 0.0f);

	// Specular term
	vec3 eye = normalize(-passPosition);
	vec3 reflection = normalize(reflect(-lightDir, passNormal));
	float cos_psi_n = pow(max(dot(reflection, eye), 0.0f), 15);

	//sum up colors
	frag_Color.rgb = baseColor;
	frag_Color.rgb += diffColor * cos_phi * lightColor;
	frag_Color.rgb += specColor * cos_psi_n * lightColor;
	frag_Color.a= 1.0f;
}