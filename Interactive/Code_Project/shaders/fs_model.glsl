#version 410 core

out vec4 color;

in VS_OUT
{
    vec4 fragPos;
    vec4 normals;
    vec2 tc;
} fs_in;

uniform sampler2D tex;
uniform mat4 model_matrix;

uniform mat4 lightPosition; // light positions
uniform vec4 viewPosition;

uniform vec4  ia;         // Ambient colour
uniform float ka;         // Ambient constant
uniform mat4  id;         // diffuse colours
uniform float kd;         // Diffuse constant
uniform mat4  is;         // specular colours
uniform float ks;         // specular constant
uniform float shininess;  // shininess constant

void main(void){
  
  vec3 sum = vec3(0.0);

  for (int i = 0; i < 4; i++) {
    // Diffuse
    vec4 lightDir = normalize(lightPosition[i] - fs_in.fragPos);
    float diff = max(dot(normalize(fs_in.normals), lightDir), 0.0);
    vec4 diffuse = diff * id[i];

    // Specular
    vec4 viewDir = normalize(viewPosition - fs_in.fragPos);
    vec4 reflectDir = reflect(-lightDir, normalize(fs_in.normals));
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    sum = vec3(sum.rgb + kd * id[i].rgb * diffuse.rgb + ks * is[i].rgb * spec);

  }
  
  // Light
  color = vec4(ka * ia.rgb + sum.rgb, 1.0) * texture(tex, fs_in.tc);
}