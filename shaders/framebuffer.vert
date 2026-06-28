#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 auv;

out vec2 uv;

uniform mat4 uViewProjection;
uniform vec4 uTransform; 
uniform vec4 uUv;
uniform float uRotation;

void main() {
    uv = mix(uUv.xy, uUv.zw, auv);
    
    float c = cos(uRotation);
    float s = sin(uRotation);
    vec2 scaled = aPos * uTransform.zw;

    vec2 rotated = vec2(
        scaled.x * c - scaled.y * s,
        scaled.x * s + scaled.y * c
    );

    gl_Position = uViewProjection * vec4(rotated + uTransform.xy, 0.0, 1.0);
}
