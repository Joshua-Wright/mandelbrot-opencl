//
// Created by j0sh on 4/13/18.
//

#ifndef MANDELBROT_OPENCL_SHADERS_H
#define MANDELBROT_OPENCL_SHADERS_H

#include <string>

std::string fragment_shader = R"'''(
#version 330
uniform sampler2D tex;
in vec2 texcoord;
out vec4 fragColor;
void main() {
    //fragColor = texture2D(tex,texcoord);
    fragColor = vec4(texcoord.x*2,texcoord.y,0.5,1);
}
)'''";

std::string vertex_shader = R"'''(
#version 330
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
uniform mat4 matrix;
out vec2 texcoord;
void main() {
    texcoord = tex;
    gl_Position = matrix * vec4(pos,1.0);
}
)'''";


#endif //MANDELBROT_OPENCL_SHADERS_H
