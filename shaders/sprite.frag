#version 330 core

in vec2 uv;
in vec4 color;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform int uSpritePixelPerfect;

void main() {
    vec2 sampleUv = uv;
    if (uSpritePixelPerfect != 0) {
        vec2 texturePixels = vec2(textureSize(uTexture, 0));
        vec2 texel = clamp(floor(uv * texturePixels),
            vec2(0.0), texturePixels - vec2(1.0));
        sampleUv = (texel + vec2(0.5)) / texturePixels;
    }
    FragColor = texture(uTexture, sampleUv) * color;
}
