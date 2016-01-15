#version 420 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texcoord;
uniform vec4 spriteFrame;
out vec4 Color;
out vec2 tex_coord;
uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

void main() {
    Color = vec4(color.x, color.y, color.z, 1.0);
    tex_coord = texcoord;
    gl_Position = g_projection * g_view * g_model * vec4(position, 0.0, 1.0);
}
