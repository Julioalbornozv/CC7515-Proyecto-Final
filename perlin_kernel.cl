float fade(float t){
    return t*t*t*(t*(t*6.0 - 15.0) + 10.0);
    }
        
__kernel void PerlinNoise(__global float2* gradients, __global float* heightmap, int gradWidth, int gradHeight, int hmapWidth, int hmapHeight, int octaveID){
    
     // Map this work-item location with the corresponding heightmap coordinates
   
    float frequency = pow(2.0f, octaveID);
    float amplitude = pow(0.99f, octaveID);
    int hmapSize = (hmapWidth * hmapHeight);
    int gradSize = (gradWidth * gradHeight);
    
    for (int pixelID = get_global_id(0); pixelID < hmapSize; pixelID += get_global_size(0)){
        
        float hx = (pixelID % hmapWidth);
        float hy = (pixelID / hmapWidth);
        
        float2 p = (float2)((float)hx/(float)gradWidth, (float)hy/(float)gradHeight)*frequency;
        
        float2 w = p - floor(p);
        
        float2 p0 = floor(p);
        float2 p1 = p0 + (float2)(1.0f, 0.0f);
        float2 p2 = p0 + (float2)(0.0f, 1.0f);
        float2 p3 = p0 + (float2)(1.0f, 1.0f);
        
        float2 g0 = gradients[((int)(p0.x + gradWidth*p0.y)%gradSize)];
        float2 g1 = gradients[((int)(p1.x + gradWidth*p1.y)%gradSize)];
        float2 g2 = gradients[((int)(p2.x + gradWidth*p2.y)%gradSize)];
        float2 g3 = gradients[((int)(p3.x + gradWidth*p3.y)%gradSize)];
        
        float p0p1 = (1.0 - fade(w.x)) * dot(g0, (p - p0)) + fade(w.x) * dot(g1, (p - p1)); /* between upper two lattice points */
        float p2p3 = (1.0 - fade(w.x)) * dot(g2, (p - p2)) + fade(w.x) * dot(g3, (p - p3)); /* between lower two lattice points */
        
        heightmap[pixelID] += ((1.0 - fade(w.y)) * p0p1 + fade(w.y) * p2p3)*amplitude;
        }
    }