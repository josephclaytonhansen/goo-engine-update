void node_shader_info(vec3 position, vec3 normal,
out vec4 half_light, out float shadows, out float self_shadows, out vec4 ambient)
{
    // This calls the EEVEE calc_shader_info function
    calc_shader_info(position, normal, half_light, shadows, self_shadows, ambient);
}

void node_shader_info_light_groups(vec3 position, vec3 normal, vec4 light_groups, vec4 light_group_shadows,
out vec4 half_light, out float shadows, out float self_shadows, out vec4 ambient)
{
    // Convert float vectors to ivec4 using floatBitsToInt for custom light groups
    ivec4 ilight_groups = ivec4(floatBitsToInt(light_groups.x), 
                                floatBitsToInt(light_groups.y),
                                floatBitsToInt(light_groups.z), 
                                floatBitsToInt(light_groups.w));
    ivec4 ilight_group_shadows = ivec4(floatBitsToInt(light_group_shadows.x),
                                       floatBitsToInt(light_group_shadows.y),
                                       floatBitsToInt(light_group_shadows.z),
                                       floatBitsToInt(light_group_shadows.w));
    
    // This calls the EEVEE calc_shader_info function with custom light groups
    calc_shader_info(position, normal, ilight_groups, ilight_group_shadows, half_light, shadows, self_shadows, ambient);
}