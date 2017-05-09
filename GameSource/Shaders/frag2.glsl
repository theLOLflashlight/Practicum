#version 330 core

in vec2 vTexCoord;

uniform vec4      uColor;
uniform sampler2D uTexture;

layout(location = 0) out vec4 out_Color;

void main()
{
    out_Color = uColor * texture( uTexture, vTexCoord );
}
