#pragma BLENDER_REQUIRE(gpu_shader_common_math_utils.glsl)

// YUV conversion matrices
const mat3 YUVFromRGB = mat3(
    vec3(0.299, -0.14713, 0.615),
    vec3(0.587, -0.28886, -0.51499),
    vec3(0.114, 0.436, -0.10001)
);

const mat3 RGBFromYUV = mat3(
    vec3(1.0, 1.0, 1.0),
    vec3(0.0, -0.394, 2.03211),
    vec3(1.13983, -0.580, 0.0)
);

float extractLuma(vec3 c)
{
    return c.r * 0.299 + c.g * 0.587 + c.b * 0.114;
}

void node_sharpen(vec4 color, vec3 vector, float gain, vec3 img_size, out vec4 result)
{
    // Note: This is a procedural sharpen effect for material shaders
    // For true texture sharpen, use compositor nodes instead
    
    vec3 yuv = YUVFromRGB * color.rgb;
    
    vec2 imgSize = img_size.xy;
    
    float accumY = 0.0;
    
    // Apply sharpening kernel using texture coordinates for variation
    for(int i = -1; i <= 1; ++i) {
        for(int j = -1; j <= 1; ++j) {
            vec2 offset = vec2(float(i), float(j)) / imgSize;
            vec2 sample_coord = vector.xy + offset;
            
            // Create procedural variation based on coordinates
            vec3 sample_color = color.rgb;
            
            if (i != 0 || j != 0) {
                // Apply coordinate-based variation to simulate neighboring pixels
                float coord_variation = sin(sample_coord.x * 200.0) * cos(sample_coord.y * 200.0) * 0.05;
                sample_color *= (1.0 + coord_variation);
            }
            
            float s = extractLuma(sample_color);
            float notCentre = min(float(i*i + j*j), 1.0);
            accumY += s * (9.0 - notCentre * 10.0);
        }
    }
    
    accumY /= 9.0;
    
    // Apply gain
    accumY = (accumY + yuv.x) * gain;
    
    // Convert back to RGB
    vec3 sharpened_rgb = RGBFromYUV * vec3(accumY, yuv.y, yuv.z);
    
    result = vec4(sharpened_rgb, color.a);
}
