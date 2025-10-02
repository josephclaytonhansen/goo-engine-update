#pragma BLENDER_REQUIRE(gpu_shader_material_common.glsl)
#pragma BLENDER_REQUIRE(gpu_shader_common_math_utils.glsl)
#pragma BLENDER_REQUIRE(gpu_shader_common_math_constants.glsl)

/* Gabor Noise Implementation for Blender */

/* PRNG structure and functions */
struct noise_prng {
    uint x_;
};

void noise_prng_srand(inout noise_prng prng, uint s) {
    prng.x_ = s;
}

uint noise_prng_rand(inout noise_prng prng) {
    return prng.x_ *= 3039177861u;
}

float noise_prng_uniform_0_1(inout noise_prng prng) {
    return float(noise_prng_rand(prng)) / 4294967295.0;
}

vec2 noise_prng_rand2_1_1(inout noise_prng prng) {
    return -1.0 + 2.0 * vec2(noise_prng_uniform_0_1(prng), noise_prng_uniform_0_1(prng));
}

uint hash_gabor(uint x) {
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = ((x >> 16) ^ x);
    return x;
}

/* Gabor kernel function */
float gabor_kernel(vec2 x, float frequency, vec2 direction, float weight, bool anisotropic) {
    float r = length(x);
    if (anisotropic) {
        return exp(-M_PI * r * r) * cos(2.0 * M_PI * frequency * dot(x, direction) + weight * M_PI);
    } else {
        return exp(-M_PI * r * r) * cos(2.0 * M_PI * frequency * r);
    }
}

/* Generate jittered center for regular splat placement */
vec2 jittered_center(int i, int splats) {
    int splats_sqrt = int(sqrt(float(splats)));
    vec2 d = vec2(float(splats_sqrt));
    vec2 loc = vec2(0.0);
    loc = loc / 2.0 + 0.5;
    loc /= d;
    loc += vec2(float(i / splats_sqrt), float(i % splats_sqrt)) / d;
    loc = 2.0 * (loc - 0.5);
    return loc;
}

void node_texture_gabor(
    vec3 vector,
    float scale,
    float frequency,
    float anisotropy,
    float orientation,
    float intensity,
    float jittering,
    float splats_per_cell,
    float anisotropic_mode,
    float jittering_mode,
    out float fac,
    out vec4 color
)
{
    /* Input processing */
    vec2 U = vector.xy * max(scale, 0.001);
    
    /* Gabor noise parameters */
    float tile_size = max(30.0 / max(scale, 0.001), 1.0);
    float tile_count = 1000.0 / tile_size; // Reasonable grid size
    int splats = int(clamp(splats_per_cell * 16.0, 1.0, 64.0));
    
    /* Direction vector for anisotropic noise */
    float angle = orientation * 2.0 * M_PI;
    vec2 direction = vec2(cos(angle), sin(angle));
    
    /* Find current cell coordinates */
    ivec2 ccell = ivec2(U / tile_size);
    
    float result = 0.0;
    noise_prng prng;
    
    /* Loop over adjacent cells (3x3 neighborhood) */
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            ivec2 disp = ivec2(dx, dy);
            ivec2 cell = ccell + disp;
            
            /* Wrap cell coordinates for tiling */
            ivec2 nc = ivec2(mod(vec2(cell), tile_count));
            
            /* Center of this cell */
            vec2 center = tile_size * (vec2(cell) + 0.5);
            
            /* Seed PRNG for this cell */
            uint seed = hash_gabor(uint(nc.x * int(tile_count) + nc.y + 1));
            noise_prng_srand(prng, seed);
            
            /* Generate splats in this cell */
            for (int i = 0; i < splats; i++) {
                vec2 splat_offset;
                
                if (jittering_mode > 0.5) {
                    /* Jittered placement */
                    splat_offset = tile_size * 0.5 * noise_prng_rand2_1_1(prng);
                } else {
                    /* Regular grid placement */
                    splat_offset = tile_size * 0.5 * jittered_center(i, splats);
                }
                
                /* Splat location */
                vec2 splat_pos = center + splat_offset;
                
                /* Random weight for this splat */
                float weight = 2.0 * noise_prng_uniform_0_1(prng) - 1.0;
                
                /* Distance to splat */
                vec2 x = (U - splat_pos) / tile_size;
                
                /* Add kernel contribution */
                bool use_anisotropic = anisotropic_mode > 0.5;
                result += weight * gabor_kernel(x, frequency, direction, weight, use_anisotropic);
            }
        }
    }
    
    /* Normalize and map to [0, 1] */
    float variance = 2.0 * sqrt(float(splats) * 0.25 * exp(-2.0 * M_PI * frequency * frequency / (tile_size * tile_size)));
    result = 0.5 * result / variance + 0.5;
    result = clamp(result, 0.0, 1.0);
    
    /* Apply intensity */
    result = mix(0.5, result, intensity);
    
    /* Output */
    fac = result;
    color = vec4(result, result, result, 1.0);
}