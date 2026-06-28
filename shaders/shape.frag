#version 330 core

in vec2 localPos;
out vec4 FragColor;

uniform vec4 uColor;
uniform int uShapeType;

void main() {
    if (uShapeType == 1 && dot(localPos * 2.0, localPos * 2.0) > 1.0) {
        discard;
    }

    FragColor = uColor;
}
