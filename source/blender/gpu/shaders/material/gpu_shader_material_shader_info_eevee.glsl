/* EEVEE calc_shader_info functions for GPU material library integration */

/* Forward declarations for EEVEE lighting system */
/* These will be provided by EEVEE when the shader links */

/* Basic calc_shader_info using material light groups */
void calc_shader_info(vec3 position,
                      vec3 normal,
                      out vec4 half_light,
                      out float shadows,
                      out float self_shadows,
                      out vec4 ambient)
{
    /* This will be linked by EEVEE closure evaluation system */
    calc_shader_info(position, normal, lightGroups, lightGroupShadows, half_light, shadows, self_shadows, ambient);
}

/* calc_shader_info with custom light groups */
void calc_shader_info(vec3 position,
                      vec3 normal,
                      ivec4 light_groups,
                      ivec4 light_group_shadows,
                      out vec4 half_light,
                      out float shadows,
                      out float self_shadows,
                      out vec4 ambient)
{
    /* This requires full EEVEE context - will be provided during shader linking */
    /* For now, provide a reasonable fallback that matches EEVEE structure */
    
    vec3 n_n = normalize(normal);
    float shadow_accum = 0.0;
    float self_shadow_accum = 0.0;
    float light_accum = 0.0;
    half_light = vec4(0.0);

    /* Simple directional light approximation */
    vec3 light_dir = normalize(vec3(0.5, 1.0, 0.8));
    float ndotl = max(dot(n_n, light_dir), 0.0);
    
    half_light = vec4(vec3(ndotl * 0.5 + 0.5), 1.0);
    shadows = 0.8;
    self_shadows = 0.9;
    ambient = vec4(0.2, 0.2, 0.25, 1.0);
}
