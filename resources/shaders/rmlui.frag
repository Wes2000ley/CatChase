#version 330 core
in vec4 vCol;
in vec2 vUV;
uniform sampler2D uTexture;
out vec4 oCol;

void main() {
    vec4 tex = texture(uTexture, vUV);
    oCol = vCol * tex;                      // premultiplied alpha stays premult
}
