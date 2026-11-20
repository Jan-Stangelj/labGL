#version 460 core

#extension GL_ARB_bindless_texture : require

layout(binding = 2, std140) uniform textureBuffer {
    uvec2 textureHandle;
};

layout(location = 0) in vec2 uvIn;

layout(location = 0) out vec4 color;

layout(location = 0) uniform sampler2D texture0;

void main() {
    color = vec4(texture(sampler2D(textureHandle), uvIn).rgb, 1.0);
}