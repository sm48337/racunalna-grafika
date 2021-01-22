#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

uniform mat4 projection;

void main() {
    TexCoords = texCoords;
    gl_Position = projection * vec4(vertex.xyz, 1.0);
}
