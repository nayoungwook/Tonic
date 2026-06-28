#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 auv;
layout(location = 2) in vec4 aTransform;
layout(location = 3) in vec4 aColor;
layout(location = 4) in vec4 aUv;
layout(location = 5) in float aRotation;

out vec2 uv;
out vec4 color;

uniform mat4 uViewProjection;

void main() {
    uv = mix(aUv.xy, aUv.zw, auv);
    color = aColor;
    float c = cos(aRotation);
    float s = sin(aRotation);
    vec2 scaled = aPos * aTransform.zw;
    vec2 rotated = vec2(
        scaled.x * c - scaled.y * s,
        scaled.x * s + scaled.y * c
    );
    gl_Position = uViewProjection * vec4(rotated + aTransform.xy, 0.0, 1.0);
}
