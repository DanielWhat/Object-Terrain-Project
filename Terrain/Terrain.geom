#version 400

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 mvpMatrix;
vec3 light_point = vec3(0, 100, 60);

out float l_dot_n;
vec3 normal_vector;
vec4 plane_vector_1;
vec4 plane_vector_2;
vec3 light_vector;

void main()
{
    plane_vector_1 = gl_in[0].gl_Position - gl_in[1].gl_Position;
    plane_vector_2 = gl_in[0].gl_Position - gl_in[2].gl_Position;

    normal_vector = normalize(vec3(plane_vector_1.y * plane_vector_2.z - plane_vector_2.y * plane_vector_1.z,
                                 -(plane_vector_1.x * plane_vector_2.z - plane_vector_2.x * plane_vector_1.z),
                                   plane_vector_1.x * plane_vector_2.y - plane_vector_2.x * plane_vector_1.y));

    int i;
    for (i = 0; i < gl_in.length; i++) {
        gl_Position =  mvpMatrix * gl_in[i].gl_Position;
        light_vector = normalize(light_point - gl_in[i].gl_Position.xyz);
        l_dot_n = dot(normal_vector, light_vector);
        EmitVertex();
    }
    EndPrimitive();
}
