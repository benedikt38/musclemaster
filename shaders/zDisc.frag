#version 450 core

in vec3 passPosition;
in vec3 passNormal;
out vec4 frag_Color;
void main()  
{       
	vec4 lightPosition = vec4(0.0, 100.0, 10.0, 1.0);
	vec3 color = vec3(0.0, 0.0, 1.0);
	vec3 ambientLight = vec3(0.2, 0.2, 0.2);
	vec3 specularColor = vec3(0.5, 0.5, 0.3);
	//Diffuse
	vec3 lightVector = normalize(lightPosition.xyz - passPosition);
	float cosPhi = max(dot(passNormal, lightVector), 0.0);
	//specular 
	//vec3 eye = normalize(-passPosition); 
	//vec3 reflection = normalize(reflect(-lightVector, passNormal));
	//float cosPsi_n = pow(max(dot(reflection, eye), 0.0f),2.0f); 
	frag_Color.a = 1.0f; 
	//frag_Color.rgb = color * ambientLight;
	//frag_Color.rgb += color * cosPhi;  
	//frag_Color.rgb += specularColor * cosPsi_n;  
	frag_Color.rgb = color * ambientLight + color * cosPhi;
}