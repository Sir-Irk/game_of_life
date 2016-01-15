#version 430 
#extension GL_ARB_shader_storage_buffer_object : require

//layout(std430, binding=1) buffer colorMods {
//	vec4 colorMods[];
//};

in vec4 Color;
in vec2 tex_coord;
in flat int colorMod;
out vec4 outColor;
uniform sampler2D texA;
void main() {
    outColor = texture(texA, tex_coord) * (Color * colorMod);
//    outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
