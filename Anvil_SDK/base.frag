#version 440 core
out vec4 FragColor;

in vec2 TexCoords; 
in vec3 Normal;
uniform sampler2D ourTexture;

void main() {
    FragColor = texture(ourTexture, TexCoords);
}