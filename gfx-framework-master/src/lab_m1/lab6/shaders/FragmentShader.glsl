#version 330

in vec3 frag_color;
in vec3 frag_normal;
in vec2 frag_texcoord;
uniform float elapsed_time_color;

layout(location = 0) out vec4 out_color;

void main()
{
    vec3 n = normalize(frag_normal);
    out_color = vec4(n, 1.0) * sin(elapsed_time_color);
}