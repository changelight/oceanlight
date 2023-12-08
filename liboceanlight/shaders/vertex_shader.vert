#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_texcoord;
layout(location = 0) out vec3 fragment_color;
layout(location = 1) out vec2 frag_texcoord;
layout(binding = 0) uniform uniform_buffer_object
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position, 0.0, 1.0);
    fragment_color = in_color;
    frag_texcoord = in_texcoord;
}
