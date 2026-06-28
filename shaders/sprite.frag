#version 330 core

in vec2 uv;
in vec4 color;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    vec2 sampleUv = uv;

    FragColor = texture(uTexture, sampleUv) * color;
}
