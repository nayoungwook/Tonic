#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 auv;

out vec2 uv;

uniform mat4 uModel;
uniform mat4 uViewProjection;

void main() {
    uv = auv;
    gl_Position = uViewProjection * uModel * vec4(aPos, 0.0, 1.0);
}