#version 400

in vec2 vTexCoord;
in vec2 vTexOffset;
in vec2 vZoom;

uniform vec4      uColor;
uniform sampler2D uTexture;

layout(location = 0) out vec4 out_Color;

void main()
{
    out_Color = uColor * texture( uTexture, fract( vTexCoord ) * vZoom + vTexOffset );
}