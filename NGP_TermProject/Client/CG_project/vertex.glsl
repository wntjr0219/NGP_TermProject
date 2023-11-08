#version 330


layout(location = 0)
in vec3 vPos; 
layout(location = 1)
in vec3 vNormal; 
layout(location = 2)
in vec2 vTexCoord;

//out vec3 out_Color; //--- 프래그먼트 세이더에게 전달

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 modelTransform;
uniform mat4 projectionTransform;

void main(void)
{
gl_Position = projectionTransform * modelTransform * vec4(vPos, 1.0);

FragPos = vec3(modelTransform * vec4(vPos, 1.0));
Normal = vNormal;
TexCoord = vTexCoord;
}
