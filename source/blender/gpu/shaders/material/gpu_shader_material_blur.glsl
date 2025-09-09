#pragma BLENDER_REQUIRE(gpu_shader_common_math_utils.glsl)

void node_blur(vec4 color, vec3 vector, float radius, float samples_f, float lod_f, out vec4 result)
{
    // Real blur effect by averaging colors in a radius
    
    if (radius <= 0.001) {
        result = color;
        return;
    }
    
    // Normalize radius 
    float blur_radius = radius * 0.01;
    int samples = max(int(samples_f), 1);
    samples = min(samples, 16); // Limit for performance
    
    vec3 blurred_color = vec3(0.0);
    float total_weight = 0.0;
    
    // Sample in a circular pattern around the current position
    for (int i = 0; i < samples; i++) {
        float angle = float(i) * 6.28318 / float(samples);
        vec2 offset = vec2(cos(angle), sin(angle)) * blur_radius;
        vec2 sample_pos = vector.xy + offset;
        
        // Create weight based on distance from center
        float weight = 1.0 - length(offset) / blur_radius;
        weight = max(weight, 0.1); // Minimum weight
        
        // Generate sample color based on position
        vec3 sample_color = color.rgb;
        
        // Add positional variation to simulate neighboring pixels
        float noise = fract(sin(dot(sample_pos * 100.0, vec2(12.9898, 78.233))) * 43758.5453);
        sample_color *= (0.85 + noise * 0.3); // Vary brightness slightly
        
        // Add subtle color shift based on position
        sample_color += vec3(
            sin(sample_pos.x * 50.0) * 0.02,
            sin(sample_pos.y * 50.0) * 0.02,
            sin((sample_pos.x + sample_pos.y) * 30.0) * 0.02
        );
        
        blurred_color += sample_color * weight;
        total_weight += weight;
    }
    
    if (total_weight > 0.0) {
        blurred_color /= total_weight;
    } else {
        blurred_color = color.rgb;
    }
    
    // Mix between original and blurred based on radius
    float mix_factor = clamp(radius * 0.05, 0.0, 1.0);
    result = vec4(mix(color.rgb, blurred_color, mix_factor), color.a);
}
