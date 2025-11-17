#version 330

in vec4 vPosition;
in vec4 vNormal;

uniform mat4 modelview;
uniform mat4 projection;
uniform mat3 normalmatrix;

out vec3 fNormal;
out vec3 fPosition;

void main()
{
    vec4 eyePosition = modelview * vPosition;
    gl_Position = projection * eyePosition;
    fPosition = eyePosition.xyz;
    fNormal = normalize(normalmatrix * vNormal.xyz);
}
