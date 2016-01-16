#version 430 
#extension GL_ARB_shader_storage_buffer_object : require

in vec4 Color;
in flat int colorMod;
out vec4 outColor;

void main() {
    outColor = Color * colorMod;
}
