#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//#extension GL_ARB_compute_shader : enable;
//#extension GL_ARB_shader_storage_buffer_object : enable;

uniform float delta; 
uniform float lerp;
uniform float cameraX;
uniform float cameraY; 
uniform float zoom; 
uniform float scalePoints; 

layout (std430, binding = 4) buffer pos {
    vec2 positions[]; 
};

layout(std430, binding = 5) buffer meta {
    vec4 metaballs[];
}; 

layout(std430, binding = 6) buffer outputpositions {
    vec2 outputPositions[];
};

float computeAtXY(float x, float y){
    float sum = 0; 
    for(int i = 0; i < metaballs.length(); i++){
        float dist = distance(metaballs[i].xy, vec2(x, y)) + 0.0001;
        sum += inversesqrt(pow(metaballs[i].x - x, 2) + pow(metaballs[i].y - y, 2)) * metaballs[i].z;
    }

    if (lerp == 1 && sum > 1){
        return 2; 
    } else if (lerp == 1 && sum < 1){
        return 0; 
    }
    return sum; 
}

void main(){
    uint gid = gl_GlobalInvocationID.x;
    bool tl, tr, bl, br, c; 

    float xPos = positions[gid].x/zoom + cameraX; 
    float yPos = positions[gid].y/zoom + cameraY; 

    // Setting the default to not visible 
    outputPositions[gid*4 + 3] = vec2(0, 0); 
    outputPositions[gid*4 + 2] = vec2(0, 0);
    outputPositions[gid*4 + 1] = vec2(0, 0); 
    outputPositions[gid*4 + 0] = vec2(0, 0);

    float values[5] = {0, 0, 0, 0, 0};

    values[0] = computeAtXY(xPos + delta/2, yPos + delta/2);
    values[1] = computeAtXY(xPos - delta/2, yPos + delta/2);
    values[2] = computeAtXY(xPos + delta/2, yPos - delta/2);
    values[3] = computeAtXY(xPos - delta/2, yPos - delta/2);
    // values[4] = computeAtXY(xPos, yPos);

    // TESTING POINTS OF THE FUNCTION
    if (values[4] > 1){
        c = true;  
    } else {
        c = false; 
    }
    // Top right 
    if (values[0] > 1){
        tr = true;  
    } else {
        tr = false; 
    }
    // Top left 
    if (values[1] > 1){
        tl = true;  
    } else {
        tl = false; 
    }
    // Bottom right
    if (values[2] > 1){
        br = true;  
    } else {
        br = false; 
    }
    // Bottom left 
    if (values[3] > 1){
        bl = true;  
    } else {
        bl = false; 
    }

    // DOING CASEWORK FOR LINE DRAWING 
    int result = 0;
    
    if (tr) result |= 1 << 0; // set bit 0      000X
    if (tl) result |= 1 << 1; // set bit 1      00X0
    if (br) result |= 1 << 2; // set bit 2      0X00
    if (bl) result |= 1 << 3; // set bit 3      X000

    // Casework
    // 1000 : 8
    // This means that the bottom left is the only one that is inside the metaball

    // 0100 : 4
    // This means that the bottom right is the only one that is inside the metaball

    // 0010 : 2
    // This means that the top left is the only one that is inside the metaball

    // 0001 : 1
    // This means that the top right is the only one that is inside the metaball

    if (result == 4){
        // Marching cubes
        float p = (1 - values[3]) / (values[2] - values[3]);
        outputPositions[gid*4] = vec2(p * delta + xPos - delta/2, yPos - delta/2);
        p = (1 - values[0]) / (values[2] - values[0]);
        outputPositions[gid*4 + 1] = vec2(xPos + delta/2, -p * delta +  yPos + delta/2);
    }
    if (result == 8){
        float p = (1 - values[1]) / (values[3] - values[1]);
        outputPositions[gid * 4] = vec2(xPos - delta/2, -p * delta +  yPos + delta/2);
        p = (1 - values[2]) / (values[3] - values[2]);
        outputPositions[gid*4 + 1] = vec2(xPos + delta/2 - p*delta, yPos - delta/2);
    }
    if (result == 2){
        float p = ( 1 - values[0]) / (values[1] - values[0]);
        outputPositions[gid*4] = vec2(-p * delta + xPos + delta/2, yPos + delta/2);
        p = (1 - values[3]) / (values[1] - values[3]);
        outputPositions[gid*4 + 1] = vec2(xPos - delta/2, p * delta +  yPos - delta/2);
    }
    if (result == 1){
        float p = (1 - values[1])/(values[0] - values[1]);
        outputPositions[gid * 4] = vec2(p * delta + xPos - delta/2, yPos + delta/2);
        p = (1 - values[2])/(values[0] - values[2]);    
        outputPositions[gid*4 + 1] = vec2(xPos + delta/2, yPos - delta/2 + p * delta);
    }

    // Casework all single lines 
    // 1100 : 12 
    // This means that the bottom left and bottom right are inside the metaball
    if (result == 12){
        float p = (1 - values[1]) / (values[3] - values[1]);
        outputPositions[gid * 4] = vec2(xPos - delta/2, -p * delta +  yPos + delta/2);
        p = (1 - values[0]) / (values[2] - values[0]);
        outputPositions[gid*4 + 1] = vec2(xPos + delta/2, yPos + delta/2 - p * delta);
    }

    // 1010 : 10
    // This means that the top left and bottom leftare inside the metaball
    if (result == 10){
        float p = (1 - values[0]) / (values[1] - values[0]);
        outputPositions[gid*4] = vec2(-p * delta + xPos + delta/2, yPos + delta/2);
        p = (1 - values[2])/(values[3] - values[2]); 
        outputPositions[gid*4 + 1] = vec2(xPos + delta/2 - p * delta, yPos - delta/2);
    }

    // 0101 : 5
    // This means that the top right and bottom right are inside the metaball
    if (result == 5){
        // Linear interpolation
        float p = (1 - values[1]) / (values[0] - values[1]);
        outputPositions[gid*4] = vec2(p * delta + xPos - delta/2, yPos + delta/2);
        p = (1 - values[3]) / (values[2] - values[3]);
        outputPositions[gid*4 + 1] = vec2(xPos - delta/2 + p * delta, yPos - delta/2);
    }

    // 0011 : 3
    // This means that the top right and top left are inside the metaball
    // HAS LERP
    if (result == 3){
        float p = (1 - values[3]) / (values[1] - values[3]);
        outputPositions[gid*4] = vec2(xPos - delta/2, p * delta +  yPos - delta/2);
        p = (1 - values[2]) / (values[0] - values[2]);
        outputPositions[gid*4 + 1] = vec2(xPos + delta/2, yPos - delta/2 + p * delta);
    }


    // Casework for corner lines where 3 values are inside the function and one is outside
    // 1110 : 14
    // This means that the bottom left, bottom right and top left are inside the metaball
    if (result == 14){
        float p = (1 - values[0])/(values[2] - values[0]);
        outputPositions[gid*4] = vec2(xPos + delta/2, -p * delta +  yPos + delta/2);
        p = (1 - values[0])/(values[1] - values[0]);
        outputPositions[gid*4 + 1] = vec2(-p * delta + xPos + delta/2, yPos + delta/2);
    }

    // 1101 : 13
    // TOP LEFT OUTSIDE METABALL
    if (result == 13){
        // Linear interpolation
        float p = (1 - values[1]) / (values[0] - values[1]);
        outputPositions[gid*4] = vec2(p * delta + xPos - delta/2, yPos + delta/2);
        p = (1 - values[1]) / (values[3] - values[1]);
        outputPositions[gid*4 + 1] = vec2(xPos - delta/2, -p * delta +  yPos + delta/2);
    }

    // 1011 : 11
    // This means that the bottom right is not in the metaball
    if (result == 11){
        float p = (1 - values[2]) / (values[0] - values[2]);
        outputPositions[gid*4] = vec2(xPos + delta/2, p * delta +  yPos - delta/2);
        p = (1 - values[2]) / (values[3] - values[2]);
        outputPositions[gid*4 + 1] = vec2(xPos + delta/2 - p * delta, yPos - delta/2);
    }

    // 0111 : 7
    // This means that the bottom left is not in the metaball 
    if (result == 7){
        float p = (1 - values[3]) / (values[1] - values[3]);
        outputPositions[gid*4] = vec2(xPos - delta/2, p * delta +  yPos - delta/2);
        p = (1 - values[3]) / (values[2] - values[3]);
        outputPositions[gid*4 + 1] = vec2(xPos - delta/2 + p * delta, yPos - delta/2);
    }
    
    // if (br == true && bl == true && tl == true && tr == true){
    //     // Marching cubes
    //     // If the entire square is inside the metaball, then just use the two lines to make an x
    //     outputPositions[gid*4] =        vec2(xPos + delta/2, yPos + delta/2);
    //     outputPositions[gid*4 + 1] =    vec2(xPos - delta/2, yPos - delta/2);
    //     outputPositions[gid*4 + 2] =    vec2(xPos - delta/2, yPos + delta/2);
    //     outputPositions[gid*4 + 3] =    vec2(xPos + delta/2, yPos - delta/2);
    // }
    // if (br == false && bl == false && tl == false && tr == false){
    //     // Marching cubes
    //     // If the entire square is inside the metaball, then just use the two lines to make an x
    //     outputPositions[gid*4] =        vec2(xPos + delta/2, yPos);
    //     outputPositions[gid*4 + 1] =    vec2(xPos - delta/2, yPos);
    //     outputPositions[gid*4 + 2] =    vec2(xPos, yPos + delta/2);
    //     outputPositions[gid*4 + 3] =    vec2(xPos, yPos - delta/2);
    // }
    
    // if (br == false && bl == false && tl == false && tr == false || true){
    //     // Marching cubes
    //     outputPositions[gid*4] = vec2(gid, metaballs.length());
    //     outputPositions[gid*4 + 1] =    vec2(computeAtXY(xPos - delta/2, yPos - delta/2), 808);
    //     outputPositions[gid*4 + 2] =    vec2(xPos, yPos);
    // }
}