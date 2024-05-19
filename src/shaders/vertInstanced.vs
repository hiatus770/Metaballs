#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aOffset;

out vec4 fColor;

uniform mat4 model; 
uniform mat4 view;
uniform mat4 projection;
uniform vec4 aColor; 

void main()
{
    gl_Position = projection * view * model * vec4(aPos + aOffset, 1.0);
    fColor = aColor; 
    // fColor = vec4(0.5f + 0.5*sin(aOffset.x/10), 0.5f + 0.5*sin(aOffset.y/10), 0.5f + 0.5*sin(aOffset.z/10), 0.0f); 
}