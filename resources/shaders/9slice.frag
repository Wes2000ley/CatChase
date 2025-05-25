#version 330 core
in  vec2 TexCoords;          // ← must match the *exact* name used in 9slice.vert
out vec4 FragColor;
uniform sampler2D image;

/* DEBUG ONE-LINER ----------------------------------
 * Shows the raw texture untouched by any tints, alpha,
 * colour multipliers, or blending.
 */
void main() { FragColor = texture(image, TexCoords); }
