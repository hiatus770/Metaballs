#version 430 core
out vec4 FragColor;
  
in vec2 coordinate; 
in vec4 color; 
in vec2 position;
in vec2 center;  
in float radius; 

void main()
{
    if (length(position - center) <= radius){
        FragColor = color; 
    } else {
        FragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f); 
    }
}
