#version 400
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 pMatrix;
uniform mat4 mvMatrix;
uniform vec3 light_point;

vec3 light_vector;
vec3 normal_vector;
vec4 plane_vector_1;
vec4 plane_vector_2;

out float l_dot_n;

/* An issue i was having was that the color on the teapot was very dark (i.e you had to multiply l_dot_n by like 100 to get a proper display)
   Turns out that normalize literally normalises vec4's, it does NOT normalise xyz and leave w untouched as 1). So you need to work with vec3
   for lighting. */


void main()
{
    plane_vector_1 = gl_in[0].gl_Position - gl_in[1].gl_Position;
    plane_vector_2 = gl_in[0].gl_Position - gl_in[2].gl_Position;

    normal_vector = vec3(plane_vector_1.y * plane_vector_2.z - plane_vector_2.y * plane_vector_1.z,
                       -(plane_vector_1.x * plane_vector_2.z - plane_vector_2.x * plane_vector_1.z),
                         plane_vector_1.x * plane_vector_2.y - plane_vector_2.x * plane_vector_1.y);
    normal_vector = normalize(normal_vector);

    int i;
    for (i = 0; i < gl_in.length; i++) {
        gl_Position = pMatrix * mvMatrix * gl_in[i].gl_Position;
        light_vector = normalize(light_point - gl_in[i].gl_Position.xyz);
        l_dot_n = max(dot(normal_vector, light_vector), 0);
        EmitVertex();
    }
    EndPrimitive();
}
