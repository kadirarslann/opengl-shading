#version 150

in vec4 vPosition;
in   vec3 vNormal;
in vec4 materialcolor;
// in vec4 vColor;
// out vec4 color;

uniform mat4 model_view;
uniform mat4 projection;

uniform vec4 LightPosition;

out  vec3 fN;
out  vec3 fE;
out  vec3 fL;
out  vec4 materialColor;


void main() {

    fN = normalize(vec3(model_view*vec4(vNormal,0.0)));
    fE = -normalize(vec3(model_view*vPosition));
    fL = normalize(vec3(LightPosition-model_view*vPosition)); //light in camera coords
    materialColor = materialcolor;

    gl_Position = projection * model_view *vPosition;
    // color = vColor;
}
