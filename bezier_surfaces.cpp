#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "CubePatches.h"

using namespace std;

float* load_vertex_data(string filename);

GLuint vaoID;
int number_of_vertices;
float CDR = 3.14159265/180.0;
float* vertex_data;


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
	glBindVertexArray(vaoID);
	glDrawArrays(GL_PATCHES, 0, number_of_vertices);
	glFlush();
}



void initialise()
{
	GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, "bezier_surfaces.vert");
	GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, "bezier_surfaces.frag");
	GLuint control_shader = load_shader(GL_TESS_CONTROL_SHADER, "bezier_surfaces.cont");
	GLuint evaluation_shader = load_shader(GL_TESS_EVALUATION_SHADER, "bezier_surfaces.eval");
	GLuint geometry_shader = load_shader(GL_GEOMETRY_SHADER, "bezier_surfaces.geom");


	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glAttachShader(program, control_shader);
	glAttachShader(program, evaluation_shader);
	glAttachShader(program, geometry_shader);
	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status); //get the link status from the program and put it into "status"

	if (status == GL_FALSE) {
		GLint infolog_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infolog_length);
		GLchar* infolog = new GLchar[infolog_length + 1];
		glGetProgramInfoLog(program, infolog_length, NULL, infolog);
		fprintf(stderr, "Linker failure: %s\n", infolog);
		delete[] infolog;
	}
	glUseProgram(program); //tells GL to use the program we just created to render stuff until we specify otherwise

	glm::mat4 proj, view, projView;

	proj = glm::perspective(20.0f*CDR, 1.0f, 10.0f, 1000.0f);  //perspective projection matrix
	view = glm::lookAt(glm::vec3(0.0, 5.0, 20.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)); //view matrix
	projView = proj*view;  //Product matrix
	glm::vec4 view_vector = -glm::vec4(0.0, 5.0, 20.0, 1);

	//Make them uniform variables to give the shader's access to them;
	GLuint model_view_proj_matrix_location = glGetUniformLocation(program, "pMatrix");
	glUniformMatrix4fv(model_view_proj_matrix_location, 1, GL_FALSE, &proj[0][0]);

	GLuint model_view_matrix_location = glGetUniformLocation(program, "mvMatrix");
	glUniformMatrix4fv(model_view_matrix_location, 1, GL_FALSE, &view[0][0]);

	GLuint view_vector_location = glGetUniformLocation(program, "view_vector");
	glUniform4fv(view_vector_location, 1, &view_vector[0]);





	vertex_data = load_vertex_data("PatchFiles/PatchVerts_Teapot.txt");
	number_of_vertices = vertex_data[0];
	vertex_data++; //the first element is the number of verticies, so increment the pointer to ignore that first value

	//Load VAOs
	GLuint vboID[2];
	glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

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

	initialise();
	glutDisplayFunc(display); //use the display function defined above to refresh the display
	glutMainLoop(); //enter the program loop
	return 0;
}
