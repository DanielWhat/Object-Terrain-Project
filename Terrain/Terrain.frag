#version 330

in float l_dot_n;
in vec2 texture_coord;
in vec4 texture_weights;
uniform sampler2D brown_grass_tex;
uniform sampler2D green_grass_tex;
uniform sampler2D water_tex;
uniform sampler2D snow_tex;
uniform bool is_wireframe;

vec4 colour;

void main()
{

    colour = texture(water_tex, texture_coord) * texture_weights.x
           + texture(green_grass_tex, texture_coord) * texture_weights.y
           + texture(brown_grass_tex, texture_coord) * texture_weights.z
           + texture(snow_tex, texture_coord) * texture_weights.w;

    if (is_wireframe) {
        gl_FragColor = vec4 (0, 0.2, 0.6, 1);
    } else {
        gl_FragColor = colour * vec4(0.2, 0.2, 0.2, 1) + max(l_dot_n, 0) * colour;
    }
}
