
#version 430 core
layout (location = 0) in vec2 aPos;

uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 
uniform vec3 cameraPos; 


void main()
{ 
    gl_Position = projection * view * vec4(aPos.x, aPos.y, 0.0, 1.0);
}