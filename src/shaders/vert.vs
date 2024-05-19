#version 430 core

layout(binding = 2, std430) buffer outputpositions {
    vec2 outputPositions[];
};

uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 
uniform vec3 cameraPos; 


void main()
{ 
    vec2 bPos = outputPositions[gl_InstanceID * 2 + gl_VertexID]; 
    gl_Position = projection * view * vec4(bPos.x, bPos.y, 0.0, 1.0);
}