#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform vec2 TexCoords;
uniform sampler2D sprite;

void main() {
    color = texture(sprite, TexCoords);
} 
