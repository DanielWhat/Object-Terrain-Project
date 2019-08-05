#version 330

in float l_dot_n;
in float h_dot_n;

uniform vec4 teapot_colour;

void main()
{
    if (length(teapot_colour.xyz) < 1) { //if the colour is very dark, don't do any lighting calculations
        gl_FragColor = teapot_colour;
    } else {
        gl_FragColor = teapot_colour * vec4(0.2, 0.2, 0.2, 1) + l_dot_n * teapot_colour + pow(h_dot_n, 60) * vec4(1, 1, 1, 1);
    }
}
