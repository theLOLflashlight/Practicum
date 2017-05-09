#version 330

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec2 texCoord;

uniform mat4 uMatrix;
uniform vec4 uSprite;

out vec2 vTexCoord;

void main()
{
    gl_Position = uMatrix * vec4( vertex, 0, 1 );
    //vTexCoord = texCoord;
    vTexCoord =  vec2( (uSprite.x + texCoord.x) / uSprite.z,
                       (uSprite.y + texCoord.y) / uSprite.w );
}