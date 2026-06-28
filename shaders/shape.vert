#version 330 core

layout(location = 0) in vec2 aPos;

uniform mat4 uViewProjection;
uniform vec4 uTransform;
uniform float uRotation;

out vec2 localPos;

void main() {
    localPos = aPos;
    float c = cos(uRotation);
    float s = sin(uRotation);
    vec2 scaled = aPos * uTransform.zw;
    vec2 rotated = vec2(
        scaled.x * c - scaled.y * s,
        scaled.x * s + scaled.y * c
    );
    gl_Position = uViewProjection * vec4(rotated + uTransform.xy, 0.0, 1.0);
}
