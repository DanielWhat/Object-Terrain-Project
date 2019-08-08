#version 400

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 texture_coordinate[];

uniform mat4 mvpMatrix;
uniform float water_level;
uniform float snow_level;
vec3 light_point = vec3(0, 100, 60);

out float l_dot_n;
out vec2 texture_coord;
out vec4 texture_weights;

vec4 vertex_pnt;
vec3 normal_vector;
vec4 plane_vector_1;
vec4 plane_vector_2;
vec3 light_vector;
vec2 mid_uv_coords;

vec4 get_gl_position(vec4 gl_pos)
/* Takes the gl_Position vec4 point and returns the new gl_Position point.
 * Adjustments are made to the point for purposes of flat water display */
{
    gl_pos.y = max(gl_pos.y, water_level);
    return gl_pos;
}

vec4 get_texture_weights(vec4 gl_pos)
/* Takes a gl_Position vec4 and returns the corresponding texture weights
 * depending on the height of the point. */
{
    float max_grass_level = 1 + 2;
    float edited_snow_level = max(snow_level, max_grass_level);
    float max_dirt_level = 1 + edited_snow_level;
    vec4 grass_blending_terrain = vec4(0, 0, 1, 0);

    //water
    if (gl_pos.y <= water_level) {
        return vec4(1, 0, 0, 0);

    //grass
    } else if (gl_pos.y < 1 + 0.2) {
        return vec4(0, 1, 0, 0);

    //grass to dirt (or grass to snow if snow is low enough)
    } else if (gl_pos.y < max_grass_level) {
        float lambda = (gl_pos.y - (1 + 0.2)) / (max_grass_level - (1 + 0.2));
        return lambda * grass_blending_terrain + (1 - lambda) * vec4(0, 1, 0, 0);

    //dirt
    } else if (gl_pos.y < max_dirt_level - 1.2) {
        return vec4(0, 0, 1, 0);

    //dirt to snow
    } else if (gl_pos.y < max_dirt_level) {
        float lambda = (gl_pos.y - (max_dirt_level - 1.2)) / (max_dirt_level - (max_dirt_level - 1.2));
        return lambda * vec4(0, 0, 0, 1) + (1 - lambda) * vec4(0, 0, 1, 0);

    //snow
    } else {
        return vec4(0, 0, 0, 1);
    }
}


void main()
{
    plane_vector_1 = get_gl_position(gl_in[0].gl_Position) - get_gl_position(gl_in[1].gl_Position);
    plane_vector_2 = get_gl_position(gl_in[0].gl_Position) - get_gl_position(gl_in[2].gl_Position);

    normal_vector = normalize(vec3(plane_vector_1.y * plane_vector_2.z - plane_vector_2.y * plane_vector_1.z,
                                 -(plane_vector_1.x * plane_vector_2.z - plane_vector_2.x * plane_vector_1.z),
                                   plane_vector_1.x * plane_vector_2.y - plane_vector_2.x * plane_vector_1.y));



    mid_uv_coords = 0.5 * texture_coordinate[0] + 0.5 * texture_coordinate[1];




    int i;
    for (i = 0; i < gl_in.length; i++) {
        vertex_pnt = get_gl_position(gl_in[i].gl_Position);
        gl_Position =  mvpMatrix * vertex_pnt;
        light_vector = normalize(light_point - vertex_pnt.xyz);
        l_dot_n = dot(normal_vector, light_vector);
        texture_coord = texture_coordinate[i];
        texture_weights = get_texture_weights(vertex_pnt);
        EmitVertex();
    }
    EndPrimitive();
}
