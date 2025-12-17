#version 330

// Input
in vec3 world_position;
in vec3 world_normal;

// Uniforms for light properties
uniform vec3 light_direction; // directional/spot direction (normalized expected)
uniform vec3 light_position;
uniform vec3 eye_position;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

// Spotlight and mode
uniform int spotlight_on;
uniform vec3 spot_direction;
uniform float spot_cutoff_cos; // cosine of cutoff angle
uniform float spot_exponent;

// Secondary light
uniform vec3 light2_position;
uniform vec3 light2_color;

uniform vec3 object_color;

// Output
layout(location = 0) out vec4 out_color;

void main()
{
    // Normalize inputs
    vec3 N = normalize(world_normal);
    vec3 V = normalize(eye_position - world_position);

    // Primary light vector (for point light use position->fragment)
    vec3 L = normalize(light_position - world_position);

    // Diffuse
    float diff = max(dot(N, L), 0.0);

    // Specular (Phong reflection model)
    vec3 R = reflect(-L, N);
    float spec = 0.0;
    if (diff > 0.0) {
        spec = pow(max(dot(R, V), 0.0), float(material_shininess));
    }

    // Spotlight attenuation based on angle between L and spot_direction
    float spotFactor = 1.0;
    if (spotlight_on == 1) {
        vec3 S = normalize(-spot_direction); // spot_direction points from light into scene? use - to match L
        float cosTheta = dot(normalize(L), S);
        if (cosTheta < spot_cutoff_cos) {
            spotFactor = 0.0; // outside cone
        } else {
            spotFactor = pow(cosTheta, spot_exponent);
        }
    }

    // Secondary light contribution (simple diffuse)
    vec3 L2 = normalize(light2_position - world_position);
    float diff2 = max(dot(N, L2), 0.0);

    // Combine components
    vec3 ambient = 0.1 * object_color;
    vec3 diffuse = material_kd * diff * object_color * vec3(1.0, 1.0, 1.0) * spotFactor;
    vec3 specular = material_ks * spec * vec3(1.0);
    vec3 diffuse2 = 0.5 * diff2 * light2_color;

    vec3 result = ambient + diffuse + specular + diffuse2;
    out_color = vec4(result, 1.0);
}
