#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "CubePatches.h"

using namespace std;

GLuint vaoID;
float CDR = 3.14159265/180.0;

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
	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_SHORT, NULL);
	glFlush();
}



void initialise()
{
	GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, "CubePatches.vert");
	GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, "CubePatches.frag");

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
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
	view = glm::lookAt(glm::vec3(0.0, 5.0, 12.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)); //view matrix
	projView = proj*view;  //Product matrix

	GLuint vboID[4];

	glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    glGenVertexArrays(2, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);

    glBindVertexArray(0);

	GLuint matrixLoc = glGetUniformLocation(program, "mvpMatrix");
	glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, &projView[0][0]);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
