#version 330 core

uniform sampler2D uTexture;
uniform vec4 uColor;
uniform vec2 uSourceSize;
uniform vec2 uOutputSize;

uniform vec2 uCameraOffset;
uniform float uCameraZoom;
uniform float uCameraRotation;
uniform int uPixelPerfect;

in vec2 uv;
out vec4 FragColor;

void main() {
	vec2 sampleCoords = uv;

	if (uPixelPerfect != 0) {
		vec2 outputPixel = uv * uOutputSize - uOutputSize * 0.5;
		float safeZoom = max(abs(uCameraZoom), 0.0001);
		float angle = -uCameraRotation;
		float cosine = cos(angle);
		float sine = sin(angle);

		vec2 sourceOffset = mat2(cosine, -sine, sine, cosine) * outputPixel
				/ safeZoom;

		if (uCameraZoom < 0.0)
			sourceOffset = -sourceOffset;
		vec2 sourcePixel = uSourceSize * 0.5 + sourceOffset + uCameraOffset;

		sampleCoords = sourcePixel / uSourceSize;
	}

	FragColor = texture(uTexture, sampleCoords);
}
