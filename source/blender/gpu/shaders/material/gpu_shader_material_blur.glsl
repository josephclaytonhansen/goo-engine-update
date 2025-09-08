#pragma BLENDER_REQUIRE(gpu_shader_common_math_utils.glsl)

float gaussian_blur(vec2 offset, float sigma)
{
    return exp(-0.5 * dot(offset / sigma, offset / sigma)) / (6.28318530718 * sigma * sigma);
}

void node_blur(vec4 color, vec3 vector, float radius, float samples_f, float lod_f, out vec4 result)
{
    // Note: This is a procedural blur effect for material shaders
    // For true texture blur, use compositor nodes instead
    
    // Convert float inputs to integers
    int samples = max(int(samples_f), 1);
    int LOD = int(lod_f);
    int sLOD = 1 << LOD;
    
    // Calculate sigma based on samples
    float sigma = float(samples) * 0.25;
    
    vec4 O = vec4(0.0);
    int s = max(samples / sLOD, 1);
    
    float total_weight = 0.0;
    
    // Create a procedural blur effect based on texture coordinates
    // This simulates the visual effect of blur without true texture sampling
    for (int i = 0; i < s * s; i++) {
        vec2 d = vec2(float(i % s), float(i / s)) * float(sLOD) - float(samples) / 2.0;
        float weight = gaussian_blur(d, sigma);
        
        // Use texture coordinates to create variation
        vec2 offset = d * radius * 0.001;
        vec2 sample_coord = vector.xy + offset;
        
        // Create procedural variation based on coordinate offset
        vec4 sample_color = color;
        
        // Apply coordinate-based variation to simulate blur
        float coord_noise = sin(sample_coord.x * 100.0) * cos(sample_coord.y * 100.0) * 0.1;
        sample_color.rgb *= (1.0 + coord_noise * radius * 0.01);
        
        O += weight * sample_color;
        total_weight += weight;
    }
    
    if (total_weight > 0.0) {
        result = O / total_weight;
    } else {
        result = color;
    }
    
    // Preserve alpha
    result.a = color.a;
}
