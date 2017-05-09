#version 400

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec2 texCoord;

uniform mat4 uMatrix;
uniform vec4 uSprite;
uniform vec2 uSize;

out vec2 vTexCoord;
out vec2 vTexOffset;
out vec2 vZoom;

void main()
{
    gl_Position = uMatrix * vec4( floor( vertex ), 0, 1 );

    vTexCoord = texCoord;
    vTexOffset = uSprite.xy / uSize;
    vZoom = uSprite.zw / uSize;
}
