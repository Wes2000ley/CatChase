#version 330 core
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;       // bytes → 0-1, premultiplied
layout(location = 2) in vec2 inTexCoord;

uniform mat4 uProjection;

out vec4 vCol;
out vec2 vUV;

void main() {
    vCol = inColor;                         // already premultiplied
    vUV  = inTexCoord;
    gl_Position = uProjection * vec4(inPosition, 0.0, 1.0);
}
