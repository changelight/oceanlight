#version 450

layout(location = 0) in vec3 fragment_color;
layout(location = 1) in vec2 frag_texcoord;
layout(location = 0) out vec4 output_color;
layout(binding = 1) uniform sampler2D tex_sampler;

void main()
{
    output_color = texture(tex_sampler, frag_texcoord);
}
