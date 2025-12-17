#version 330

// Input
in vec2 texcoord;

// Uniform properties
uniform sampler2D texture_1;
uniform sampler2D texture_2;
uniform int use_texture_1;
uniform int use_texture_2;
uniform float mix_factor;
uniform int use_time_scroll;
uniform float time_value;
uniform float scroll_speed;

// Output
layout(location = 0) out vec4 out_color;

void main()
{
    vec2 coord1 = texcoord;
    if (use_time_scroll == 1)
    {
        coord1.x = fract(coord1.x + time_value * scroll_speed);
    }

    vec4 color1 = vec4(1.0);
    if (use_texture_1 == 1)
    {
        color1 = texture(texture_1, coord1);
    }

    vec4 finalColor = color1;
    if (use_texture_2 == 1)
    {
        vec4 color2 = texture(texture_2, texcoord);
        finalColor = mix(color1, color2, clamp(mix_factor, 0.0f, 1.0f));
    }

    if (finalColor.a < 0.5f)
    {
        discard;
    }

    out_color = finalColor;
}
