#version 420 core
in vec4 Color;
in vec2 tex_coord;
out vec4 outColor;
uniform sampler2D texA;
void main() {
      outColor = texture(texA, tex_coord) * Color;
//    outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
