#version 330 core
in vec2 vUV;
in vec4 vColor;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform int uUseTexture;  /* 0 = color only, 1 = textured */

void main() {
    if (uUseTexture == 1) {
        FragColor = texture(uTexture, vUV) * vColor;
    } else {
        FragColor = vColor;
    }
}
