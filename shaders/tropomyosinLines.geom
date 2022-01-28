#version 450 core

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;
in vec4 passPos_G[];
flat in float passRadius_G[];
uniform mat4 rotationMatrix;
uniform mat4 scaleWidthMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec4 passWorldPos;
out vec4 passPos;
out vec4 tangent;
out vec2 passUV;

void main() {
    vec4 line_p0 = viewMatrix * passPos_G[0];
    vec4 line_p1 = viewMatrix * passPos_G[1];
    vec4 line_p2 = viewMatrix * passPos_G[2];
    vec4 line_p3 = viewMatrix * passPos_G[3];
    
    vec4 temp_tangent_p1 = vec4(normalize((line_p2 - line_p0).xyz), 0.0f);
    vec4 temp_tangent_p2 = vec4(normalize((line_p3 - line_p1).xyz), 0.0f);
    vec4 sideways_vector_p1 = vec4(normalize(cross(normalize(line_p1.xyz), temp_tangent_p1.xyz)), 0.0f);
    vec4 sideways_vector_p2 = vec4(normalize(cross(normalize(line_p2.xyz), temp_tangent_p2.xyz)), 0.0f);

    tangent = temp_tangent_p1;
    passUV = vec2(1, 1);
    passWorldPos = passPos_G[1];
    passPos = line_p1 - passRadius_G[1] * sideways_vector_p1;
    gl_Position = projectionMatrix * passPos;
    EmitVertex();
    
    tangent = temp_tangent_p1;
    passUV = vec2(1, -1);
    passWorldPos = passPos_G[1];
    passPos = line_p1 + passRadius_G[1] * sideways_vector_p1;
    gl_Position = projectionMatrix * passPos;
    EmitVertex();
    
    tangent = temp_tangent_p2;
    passUV = vec2(-1, 1);
    passWorldPos = passPos_G[2];
    passPos = line_p2 - passRadius_G[1] * sideways_vector_p2;
    gl_Position = projectionMatrix * passPos;
    EmitVertex();
    
    tangent = temp_tangent_p2;
    passUV = vec2(-1, -1);
    passWorldPos = passPos_G[2];
    passPos = line_p2 + passRadius_G[1] * sideways_vector_p2;
    gl_Position = projectionMatrix * passPos;
    EmitVertex();

    //EndPrimitive();
}


