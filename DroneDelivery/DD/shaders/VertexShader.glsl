#version 330 core

in vec3 position;
out vec3 vertexColor;

uniform float time;
uniform float frequency;

void main() {
    vec2 noiseInput = position.xz * frequency;
    float noiseValue = snoise(noiseInput);
    vec3 newPosition = vec3(position.x, noiseValue, position.z);
    gl_Position = projection * view * model * vec4(newPosition, 1.0);

    vertexColor = mix(vec3(0.0, 0.5, 0.0), vec3(0.8, 0.8, 0.5), noiseValue);
}
