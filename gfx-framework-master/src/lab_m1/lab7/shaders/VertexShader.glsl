#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Uniforms for light properties
uniform vec3 light_position;
uniform vec3 eye_position;
uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

uniform vec3 object_color;

// Output value to fragment shader
out vec3 color;


void main()
{
    // Transform position and normal to world space
    vec3 world_pos = vec3(Model * vec4(v_position, 1.0));
    vec3 N = normalize(mat3(Model) * v_normal);
    vec3 L = normalize(light_position - world_pos);
    vec3 V = normalize(eye_position - world_pos);
    vec3 H = normalize(L + V); // Blinn-Phong

    // Ambient component
    float ambient_light = 0.25;
    vec3 ambient = ambient_light * object_color;

    // Diffuse component
    float diffuse_light = max(dot(N, L), 0.0);
    vec3 diffuse = material_kd * diffuse_light * object_color;

    // Specular component (Phong)
    float specular_light = 0.0;
    if (diffuse_light > 0.0) {
        // Phong
        vec3 R = reflect(-L, N);
        specular_light = pow(max(dot(R, V), 0.0), material_shininess);
    }
    vec3 specular = material_ks * specular_light * vec3(1.0);

    // Blinn-Phong variant (optional, can be added to specular)
    float blinn_specular = 0.0;
    if (diffuse_light > 0.0) {
        blinn_specular = pow(max(dot(N, H), 0.0), material_shininess);
    }
    // You can blend Phong and Blinn-Phong if desired:
    // specular += material_ks * blinn_specular * vec3(1.0);

    // Attenuation factor
    float distance = length(light_position - world_pos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

    // Final color
    color = (ambient + diffuse + specular) * attenuation;

    gl_Position = Projection * View * Model * vec4(v_position, 1.0);
}
