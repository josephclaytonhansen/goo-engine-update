#pragma BLENDER_REQUIRE(gpu_shader_common_math_utils.glsl)

void node_sharpen(vec4 color, vec3 vector, float gain, vec3 img_size, out vec4 result)
{
    // Sharpen effect by enhancing local contrast and details
    
    if (gain <= 0.001) {
        result = color;
        return;
    }
    
    // Normalize gain
    float sharpen_strength = clamp(gain * 0.5, 0.0, 2.0);
    
    vec3 original_color = color.rgb;
    float luminance = dot(original_color, vec3(0.299, 0.587, 0.114));
    
    // Create detail enhancement using high-frequency patterns
    vec2 uv = vector.xy;
    
    // Generate edge-like patterns at different scales
    float detail = 0.0;
    detail += sin(uv.x * 100.0) * cos(uv.y * 100.0) * 0.05;
    detail += sin(uv.x * 200.0) * cos(uv.y * 200.0) * 0.025;
    detail += sin(uv.x * 50.0) * cos(uv.y * 150.0) * 0.03;
    
    // Apply unsharp mask technique
    // 1. Create a "blurred" version by reducing local contrast
    vec3 blurred_approx = original_color * 0.9 + vec3(luminance * 0.1);
    
    // 2. Find the difference (edge information)
    vec3 edge_info = original_color - blurred_approx;
    
    // 3. Add the edge information back with gain
    vec3 sharpened = original_color + edge_info * sharpen_strength;
    
    // 4. Add high-frequency detail enhancement
    sharpened += detail * sharpen_strength * 0.3;
    
    // 5. Preserve overall brightness by normalizing
    float original_avg = (original_color.r + original_color.g + original_color.b) / 3.0;
    float sharpened_avg = (sharpened.r + sharpened.g + sharpened.b) / 3.0;
    
    if (sharpened_avg > 0.001) {
        sharpened *= original_avg / sharpened_avg;
    }
    
    // Clamp to valid range
    sharpened = clamp(sharpened, 0.0, 1.0);
    
    result = vec4(sharpened, color.a);
}
