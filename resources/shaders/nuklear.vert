#version 330 core
layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec4 Color;

out vec2 Frag_UV;
out vec4 Frag_Color;

uniform mat4 projection;

void main() {
    Frag_UV = TexCoord;
    Frag_Color = Color;
    gl_Position = projection * vec4(Position.xy, 0, 1);
}
