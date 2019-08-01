#version 330

in float l_dot_n;

void main()
{
     gl_FragColor = vec4(0, 0, 0.2, 1) + l_dot_n * vec4(0, 0, 1, 1);
}
