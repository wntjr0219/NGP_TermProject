#version 330
//--- out_Color: 버텍스 세이더에서 입력받는 색상 값
//--- FragColor: 출력할 색상의 값으로 프레임 버퍼로 전달 됨. 

//in vec3 out_Color; //--- 버텍스 세이더에게서 전달 받음

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor; //--- 색상 출력

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D outTexture;

void main(void)
{
float ambientLight = 0.5;
vec3 ambient = ambientLight * lightColor;

vec3 normalVector = normalize(Normal);
vec3 lightDir = normalize(lightPos - FragPos);
float diffuseLight = max(dot(normalVector, lightDir), 0.0);
vec3 diffuse = diffuseLight * lightColor;

int shininess = 128;
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir, normalVector);
float specularLight = max(dot(viewDir, reflectDir), 0.0);
specularLight = pow(specularLight, shininess);
vec3 specular = specularLight * lightColor;


vec3 result = (ambient + diffuse + specular) * objectColor;

FragColor = vec4(result, 1.0);
FragColor = texture(outTexture, TexCoord) * FragColor;
}
