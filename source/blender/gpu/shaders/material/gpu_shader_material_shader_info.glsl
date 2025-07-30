
/* Shader Info node implementation - self-contained like SDF functions */

void node_shader_info(vec3 position, vec3 normal,
out vec4 half_light, out float shadows, out float self_shadows, out vec4 ambient)
{
    /* Implement basic NPR lighting calculations directly */
    vec3 n_n = normalize(normal);
    
    /* Simple directional light for half_light calculation */
    vec3 light_dir = normalize(vec3(0.5, 1.0, 0.8));
    float ndotl = max(dot(n_n, light_dir), 0.0);
    half_light = vec4(vec3(ndotl * 0.5 + 0.5), 1.0);
    
    /* Basic shadow approximation */
    shadows = 0.8;
    self_shadows = 0.9;
    
    /* Simple ambient lighting */
    ambient = vec4(0.2, 0.2, 0.25, 1.0);
}

void node_shader_info_light_groups(vec3 position, vec3 normal, vec4 light_groups, vec4 light_group_shadows,
out vec4 half_light, out float shadows, out float self_shadows, out vec4 ambient)
{
    /* Light group version - same basic lighting for now */
    vec3 n_n = normalize(normal);
    
    /* Simple directional light for half_light calculation */
    vec3 light_dir = normalize(vec3(0.5, 1.0, 0.8));
    float ndotl = max(dot(n_n, light_dir), 0.0);
    half_light = vec4(vec3(ndotl * 0.5 + 0.5), 1.0);
    
    /* Basic shadow approximation */
    shadows = 0.8;
    self_shadows = 0.9;
    
    /* Simple ambient lighting */
    ambient = vec4(0.2, 0.2, 0.25, 1.0);
}
