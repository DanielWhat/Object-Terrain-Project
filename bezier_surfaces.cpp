#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "CubePatches.h"

#define LIGHT_BLUE glm::vec4(0, 0.2, 1, 1)
#define BLACK glm::vec4(0, 0, 0, 1)

using namespace std;

float* load_vertex_data(string filename);
GLuint vaoID_teapot;
GLuint vaoID_floor;

float distance_from_teapot;
float z_offset = 0;
int teapot_polygon_mode = GL_FILL;
glm::vec4 teapot_colour = LIGHT_BLUE;

float initial_velocity = 4;
float explosion_time = 0;

int number_of_vertices;
int number_of_floor_vertices;
float CDR = 3.14159265/180.0;
GLuint teapot_program;
GLuint floor_grid_program;
//glm::vec4 camera_location =


float* generate_floor_grid(int num_x_boxes, int num_y_boxes, int box_length, glm::vec3 origin)
/* Generates a floor grid of num_x_boxes by num_y_boxes. The origin gives the coordinates
 * of the bottom left of the grid. Returns a pointer to float* list where every 3 floats
 * reprisents a vertex.*/
{
	int i, j;
	float* floor_grid_vertices = new float[num_x_boxes*num_y_boxes*4*3]; /*5*5 gives the number of boxes, *4 gives the number of verticies
													   and *3 gives the number of floats*/
	for (j = 0; j < num_x_boxes; j++) {
		for (i = 0; i < num_y_boxes; i++) {
			//assign 0, 0
			//cout << j*20 << 'y' << i*20 << endl;
			floor_grid_vertices[i*12 + j*12*num_y_boxes] = j*box_length + origin.x;
			floor_grid_vertices[i*12 + j*12*num_y_boxes + 1] = origin.y;
			floor_grid_vertices[i*12 + j*12*num_y_boxes + 2] = i*box_length + origin.z;

			//assign 0, 20
			//cout << j*20 << 'y' << (i+1)*20 << endl;
			floor_grid_vertices[i*12 + j*12*num_y_boxes + 3] = j*box_length + origin.x;
			floor_grid_vertices[i*12 + j*12*num_y_boxes + 4] = origin.y;
			floor_grid_vertices[i*12 + j*12*num_y_boxes + 5] = (i+1)*box_length + origin.z;

			//assign 20, 20
			//cout << (j+1)*20 << 'y' << (i+1)*20 << endl;
			floor_grid_vertices[i*12 + j*12*num_y_boxes + 6] = (j+1)*box_length + origin.x;
			floor_grid_vertices[i*12 + j*12*num_y_boxes + 7] = origin.y;
			floor_grid_vertices[i*12 + j*12*num_y_boxes + 8] = (i+1)*box_length + origin.z;

			//assign 20, 0
			//cout << (j+1)*20 << 'y' << i*20 << endl;
			floor_grid_vertices[i*12 + j*12*num_y_boxes + 9] = (j+1)*box_length + origin.x;
			floor_grid_vertices[i*12 + j*12*num_y_boxes + 10] = origin.y;
			floor_grid_vertices[i*12 + j*12*num_y_boxes + 11] = i*box_length + origin.z;
		}
	}

	return floor_grid_vertices;
}


float* load_vertex_data(string filename)
/* Loads vertex data from the given file, and returns a float* pointing to the data/list of floats.
 * The first element in the list is the number of verticies in the list (i.e the first element
 * multiplied by 3 will give you the total number of elements in the list)*/
{
	ifstream vertex_file = ifstream(filename.c_str());

	if (!vertex_file.good()) {
		cout << "There was an error opening the given file." << endl;
	}

	int num_vertices;
	string line;
	getline(vertex_file, line);
	istringstream line_string = istringstream(line);
	line_string >> num_vertices;

	float* data_pointer = new float[num_vertices*3]; //each vertex has 3 floats, so 3 times the number of vertices
	                                                  //and we add 1 to make the first element the number of vertices in the file
	data_pointer[0] = num_vertices;

	int i = 1;
	while (getline(vertex_file, line)) {
		float x, y, z;
		line_string = istringstream(line);
		line_string >> x >> y >> z;
		data_pointer[i] = x;
		data_pointer[i+1] = y;
		data_pointer[i+2] = z;
		i += 3;
	}

	return data_pointer;
}




GLuint load_shader(GLenum shader_type, string shader_filename)
{
	ifstream shader_file = ifstream(shader_filename.c_str());

	if (!shader_file.good()) {
		cout << "Error opening the given shader file" << endl;
	}

	stringstream shader_data;
	shader_data << shader_file.rdbuf(); //returns a pointer to the streambuffer object?
	shader_file.close();
	string shader_str = shader_data.str();
	const char* shader_text = shader_str.c_str();

	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &shader_text, NULL); //loads the code in shaderTxt into the shader object we just passed it
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status); //get the compile status from the shader and put it into "status"

	if (status == GL_FALSE) {
		GLint infolog_length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolog_length);
		GLchar* info_log = new GLchar[infolog_length + 1];
		glGetShaderInfoLog(shader, infolog_length, NULL, info_log);
		const char *strShaderType = NULL; //I don't know why this is here, delete it later @@
		cerr << "Compilation has failed in shader: " << info_log << endl;
		delete[] info_log;
	}
	return shader;
}



void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //@@ look up meaning

	glm::mat4 proj, view, projView;

	proj = glm::perspective(20.0f*CDR, 1.0f, 10.0f, 1000.0f);  //perspective projection matrix
	view = glm::lookAt(glm::vec3(0.0, 5.0, 30.0 + z_offset), glm::vec3(0.0, 0.0, 0.0 + z_offset), glm::vec3(0.0, 1.0, 0.0)); //view matrix
	projView = proj*view;  //Product matrix

	distance_from_teapot = glm::length(glm::vec3(0.0, 5.0, 30.0 + z_offset)); //since teapot is at origin, camera position is a vector for distance from teapot

	//Teapot
	glUseProgram(teapot_program);

	GLuint model_view_proj_matrix_location = glGetUniformLocation(teapot_program, "pMatrix");
	glUniformMatrix4fv(model_view_proj_matrix_location, 1, GL_FALSE, &proj[0][0]);
	GLuint model_view_matrix_location = glGetUniformLocation(teapot_program, "mvMatrix");
	glUniformMatrix4fv(model_view_matrix_location, 1, GL_FALSE, &view[0][0]);
	GLuint distance_from_teapot_location = glGetUniformLocation(teapot_program, "distance_from_teapot");
	glUniform1f(distance_from_teapot_location, distance_from_teapot);
	GLuint teapot_colour_location = glGetUniformLocation(teapot_program, "teapot_colour");
	glUniform4fv(teapot_colour_location, 1, &teapot_colour[0]);
	GLuint time_location = glGetUniformLocation(teapot_program, "time");
	glUniform1f(time_location, explosion_time);

	glBindVertexArray(vaoID_teapot);
	glPolygonMode(GL_FRONT_AND_BACK, teapot_polygon_mode);
	glDrawArrays(GL_PATCHES, 0, number_of_vertices);


	//Floor Grid
	glUseProgram(floor_grid_program);

	GLuint model_view_proj_matrix_location_floor = glGetUniformLocation(floor_grid_program, "mvpMatrix");
	glUniformMatrix4fv(model_view_proj_matrix_location_floor, 1, GL_FALSE, &projView[0][0]);

	glBindVertexArray(vaoID_floor);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_QUADS, 0, number_of_floor_vertices);

	glFlush();
}


void initialise_floor_grid()
{
	GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, "bezier_surfaces_floor.vert");
	GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, "bezier_surfaces_floor.frag");

	floor_grid_program = glCreateProgram();
	glAttachShader(floor_grid_program, vertex_shader);
	glAttachShader(floor_grid_program, fragment_shader);
	glLinkProgram(floor_grid_program);

	GLint status;
	glGetProgramiv(floor_grid_program, GL_LINK_STATUS, &status); //get the link status from the program and put it into "status"

	if (status == GL_FALSE) {
		GLint infolog_length;
		glGetProgramiv(floor_grid_program, GL_INFO_LOG_LENGTH, &infolog_length);
		GLchar* infolog = new GLchar[infolog_length + 1];
		glGetProgramInfoLog(floor_grid_program, infolog_length, NULL, infolog);
		fprintf(stderr, "Linker failure: %s\n", infolog);
		delete[] infolog;
	}
	glUseProgram(floor_grid_program); //tells GL to use the program we just created to render stuff until we specify otherwise

	float* vertex_data = generate_floor_grid(10, 15, 2, glm::vec3(-10, 0, -15));
	number_of_floor_vertices = 10 * 15 * 4;

	//Load VAOs
	GLuint vboID[2];
	glGenVertexArrays(1, &vaoID_floor);
	glBindVertexArray(vaoID_floor);

	glGenVertexArrays(2, vboID); //generate a vertex array

	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]); //tell openGL that this buffer is specifying "Vertex attributes" and bind it so we can work on it
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*number_of_floor_vertices*3, vertex_data, GL_STATIC_DRAW); //bind the data in vextex_data to this vbo
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); /*set the location index to 0 (this is the index you use to
	access the data in this vbo in the shaders), tell it that each vertex is defined with 3 floats. */
	glEnableVertexAttribArray(0);  //enable the vertex positon array/vbo we just set up

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}



void initialise_teapot()
{
	GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, "bezier_surfaces.vert");
	GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, "bezier_surfaces.frag");
	GLuint control_shader = load_shader(GL_TESS_CONTROL_SHADER, "bezier_surfaces.cont");
	GLuint evaluation_shader = load_shader(GL_TESS_EVALUATION_SHADER, "bezier_surfaces.eval");
	GLuint geometry_shader = load_shader(GL_GEOMETRY_SHADER, "bezier_surfaces.geom");


	teapot_program = glCreateProgram();
	glAttachShader(teapot_program, vertex_shader);
	glAttachShader(teapot_program, fragment_shader);
	glAttachShader(teapot_program, control_shader);
	glAttachShader(teapot_program, evaluation_shader);
	glAttachShader(teapot_program, geometry_shader);
	glLinkProgram(teapot_program);

	GLint status;
	glGetProgramiv(teapot_program, GL_LINK_STATUS, &status); //get the link status from the program and put it into "status"

	if (status == GL_FALSE) {
		GLint infolog_length;
		glGetProgramiv(teapot_program, GL_INFO_LOG_LENGTH, &infolog_length);
		GLchar* infolog = new GLchar[infolog_length + 1];
		glGetProgramInfoLog(teapot_program, infolog_length, NULL, infolog);
		fprintf(stderr, "Linker failure: %s\n", infolog);
		delete[] infolog;
	}
	glUseProgram(teapot_program); //tells GL to use the program we just created to render stuff until we specify otherwise

	glm::mat4 proj, view, projView;

	proj = glm::perspective(20.0f*CDR, 1.0f, 10.0f, 1000.0f);  //perspective projection matrix
	view = glm::lookAt(glm::vec3(0.0, 5.0, 30.0 + z_offset), glm::vec3(0.0, 0.0, 0.0 + z_offset), glm::vec3(0.0, 1.0, 0.0)); //view matrix
	projView = proj*view;  //Product matrix
	glm::vec3 camera_point = glm::vec3(glm::vec3(0.0, 5.0, 30.0 + z_offset));
	glm::vec3 light_point = glm::vec3(3, 30, 100);

	//Make some uniform variables to give the shader's access to them;
	GLuint light_point_location = glGetUniformLocation(teapot_program, "light_point");
	glUniform3fv(light_point_location, 1, &light_point[0]);

	GLuint camera_point_location = glGetUniformLocation(teapot_program, "camera_point");
	glUniform3fv(camera_point_location, 1, &camera_point[0]);

	GLuint initial_velocity_location = glGetUniformLocation(teapot_program, "initial_velocity");
	glUniform1f(initial_velocity_location, initial_velocity);



	float* vertex_data = load_vertex_data("PatchFiles/PatchVerts_Teapot.txt"); //@@@ CAREFUL, THIS MAY CAUSE PROBLEMS LATER ********************************************************
	number_of_vertices = vertex_data[0];
	vertex_data++; //the first element is the number of verticies, so increment the pointer to ignore that first value

	//Load VAOs
	GLuint vboID[2];
	glGenVertexArrays(1, &vaoID_teapot);
    glBindVertexArray(vaoID_teapot);

    glGenVertexArrays(2, vboID); //generate a vertex array

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]); //tell openGL that this buffer is specifying "Vertex attributes" and bind it so we can work on it
	glPatchParameteri(GL_PATCH_VERTICES, 16); //let opngl know that our patches have 16 vertices each
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*number_of_vertices*3, vertex_data, GL_STATIC_DRAW); //bind the data in vextex_data to this vbo
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); /*set the location index to 0 (this is the index you use to
	access the data in this vbo in the shaders), tell it that each vertex is defined with 3 floats.*/
    glEnableVertexAttribArray(0);  //enable the vertex positon array/vbo we just set up

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}


void render_explosion(int value)
/* Handles the updating of the explosion effect. Calls itself to render the next frame */
{
	explosion_time += 0.03;
	if (explosion_time < 10) {
		glutTimerFunc(1000/30.0, render_explosion, 0);
	}
	glutPostRedisplay();
}


void move_camera(int key, int x, int y)
{
	if (key == GLUT_KEY_UP) {
		z_offset -= 1;
	} else if (key == GLUT_KEY_DOWN) {
		z_offset += 1;
	}
	glutPostRedisplay();

}

void toggle_teapot_wireframe (unsigned char key, int x, int y)
{
	if (key == 'w') {
		teapot_polygon_mode = (teapot_polygon_mode == GL_FILL) ? GL_LINE : GL_FILL;
		teapot_colour = (teapot_colour == LIGHT_BLUE) ? BLACK : LIGHT_BLUE;
		glutPostRedisplay();

	} else if (key == ' ') {
		render_explosion(0);
		cout << "Explosion started" << endl;
	}
}


int main (int argc, char** argv)
{
	glutInit(&argc, argv); //initialises GLUT libary and makes sure a window is ready
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH); //GLUT_RGB make sure colour is shown, GLUT_DEPTH allow for a depth buffer
	glutInitWindowSize(500, 500);
	glutCreateWindow("Bezier Surface Modeling");
	glutInitContextVersion(4, 2); //use opengl version to 4.2
	glutInitContextProfile(GLUT_CORE_PROFILE);

	//remove this later @@
	if(glewInit() == GLEW_OK)
	{
		cout << "GLEW initialization successful! " << endl;
		cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
	}
	else
	{
		cerr << "Unable to initialize GLEW  ...exiting." << endl;
		exit(EXIT_FAILURE);
	}

	initialise_teapot();
	initialise_floor_grid();
	glutSpecialFunc(move_camera);
	glutKeyboardFunc(toggle_teapot_wireframe);
	glutDisplayFunc(display); //use the display function defined above to refresh the display
	glutMainLoop(); //enter the program loop
	return 0;
}
