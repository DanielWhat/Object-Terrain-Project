//  ========================================================================
//  COSC422: Computer Graphics (2018);  University of Canterbury.
//
//  FILE NAME: Terrain.cpp
//  This is part of Assignment1 files.
//
//	The program generates and loads the mesh data for a terrain floor (100 verts, 81 elems).
//  Required files:  Terrain.vert (vertex shader), Terrain.frag (fragment shader), HeightMap1.tga  (height map)
//  ========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadTGA.h"
using namespace std;

GLuint vaoID;
GLuint theProgram;
GLuint mvpMatrixLoc;
GLuint view_matrix_loc;
GLuint water_level_loc;
GLuint snow_level_loc;
GLuint wireframe_bool_loc;
GLuint max_grass_level_loc;
GLuint is_hidden_loc;

float CDR = 3.14159265/180.0;     //Conversion from degrees to rad (required in GLM 0.9.6)

float verts[100*3];       //10x10 grid (100 vertices)
GLushort elems[81*4];       //Element array for 81 quad patches

glm::mat4 projView, view, proj;
glm::vec3 camera_position;

float z_offset;
float x_offset;
GLuint polygon_mode = GL_FILL;
bool is_wireframe = false;
bool is_hidden_crack_fix_enabled = true;

float water_level = 1;
float snow_level = 5;
float max_grass_level = 3;

GLuint texID[6];

//Generate vertex and element data for the terrain floor
void generateData()
{
	int indx, start;
	//verts array
	for(int i = 0; i < 10; i++)   //100 vertices on a 10x10 grid
	{
		for(int j = 0; j < 10; j++)
		{
			indx = 10*i + j;
			verts[3*indx] = 10*i - 45;		//x  varies from -45 to +45
			verts[3*indx+1] = 0;			//y  is set to 0 (ground plane)
			verts[3*indx+2] = -10*j;		//z  varies from 0 to -100
		}
	}

	//elems array
	for(int i = 0; i < 9; i++)
	{
		for(int j = 0; j < 9; j++)
		{
			indx = 9*i +j;
			start = 10*i + j;
			elems[4*indx] = start;
			elems[4*indx+1] = start+10;
			elems[4*indx+2] = start+11;
			elems[4*indx+3] = start+1;
		}
	}
}



void load_texture(string texture_filename, GLuint tex_id, GLuint gl_texture)
/* Takes a texture file name, a texture_id and a GL_TEXTUREi constant. It then
 *  assigns the given texture to GL_TEXTUREi and the texture_id */
{
	glActiveTexture(gl_texture);
    glBindTexture(GL_TEXTURE_2D, tex_id);
	loadTGA(texture_filename);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}



//Loads terrain texture
void loadTextures()
{
	int num_textures = 5;
	const char* textures[num_textures] = {"HeightMap1.tga", "./textures/dirt.tga", "./textures/green_grass.tga", "./textures/water.tga", "./textures/snow.tga"};
	int texture_actives[num_textures] = {GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5};
    glGenTextures(num_textures, texID);

	for (int i = 0; i < num_textures; i++) {
		load_texture(textures[i], texID[i], texture_actives[i]);
	}
}



//Loads a shader file and returns the reference to a shader object
GLuint loadShader(GLenum shaderType, string filename)
{
	ifstream shaderFile(filename.c_str());
	if(!shaderFile.good()) cout << "Error opening shader file." << endl;
	stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();
	string shaderStr = shaderData.str();
	const char* shaderTxt = shaderStr.c_str();

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderTxt, NULL);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		const char *strShaderType = NULL;
		cerr <<  "Compile failure in shader: " << strInfoLog << endl;
		delete[] strInfoLog;
	}
	return shader;
}



//Initialise the shader program, create and load buffer data
void initialise()
{
	loadTextures();
//--------Load shaders----------------------
	GLuint shaderv = loadShader(GL_VERTEX_SHADER, "Terrain.vert");
	GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "Terrain.frag");
	GLuint control_shader = loadShader(GL_TESS_CONTROL_SHADER, "Terrain.cont");
	GLuint evaluation_shader = loadShader(GL_TESS_EVALUATION_SHADER, "Terrain.eval");
	GLuint geometry_shader = loadShader(GL_GEOMETRY_SHADER, "Terrain.geom");

	GLuint program = glCreateProgram();
	glAttachShader(program, shaderv);
	glAttachShader(program, shaderf);
	glAttachShader(program, control_shader);
	glAttachShader(program, evaluation_shader);
	glAttachShader(program, geometry_shader);
	glLinkProgram(program);

	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}
	glUseProgram(program);

	mvpMatrixLoc = glGetUniformLocation(program, "mvpMatrix");
	view_matrix_loc = glGetUniformLocation(program, "view_matrix");
	water_level_loc = glGetUniformLocation(program, "water_level");
	snow_level_loc = glGetUniformLocation(program, "snow_level");
	wireframe_bool_loc = glGetUniformLocation(program, "is_wireframe");
	max_grass_level_loc = glGetUniformLocation(program, "max_grass_level");
	is_hidden_loc = glGetUniformLocation(program, "is_hidden_crack_fix_enabled");

	GLuint texLoc = glGetUniformLocation(program, "heightMap");
	glUniform1i(texLoc, 0);
	GLuint brown_grass_tex_loc = glGetUniformLocation(program, "brown_grass_tex");
	glUniform1i(brown_grass_tex_loc, 1);
	GLuint green_grass_tex_loc = glGetUniformLocation(program, "green_grass_tex");
	glUniform1i(green_grass_tex_loc, 2);
	GLuint water_tex_loc = glGetUniformLocation(program, "water_tex");
	glUniform1i(water_tex_loc, 3);
	GLuint snow_tex_loc = glGetUniformLocation(program, "snow_tex");
	glUniform1i(snow_tex_loc, 4);

//--------Compute matrices----------------------
	proj = glm::perspective(30.0f*CDR, 1.25f, 20.0f, 500.0f);  //perspective projection matrix

//---------Load buffer data-----------------------
	generateData();
	glPatchParameteri(GL_PATCH_VERTICES, 4);

	GLuint vboID[2];
	glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    glGenBuffers(2, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);

    glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}



//Display function to compute uniform values based on transformation parameters and to draw the scene
void display()
{
	camera_position = glm::vec3(0.0 + x_offset, 20.0, 22.0 + z_offset);
	view = glm::lookAt(camera_position, glm::vec3(0.0 + x_offset, 0.0, -40.0 + z_offset), glm::vec3(0.0, 1.0, 0.0)); //view matrix
	projView = proj * view;  //Product (mvp) matrix

	glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &projView[0][0]);
	glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, &view[0][0]);
	glUniform1f(water_level_loc, water_level);
	glUniform1f(snow_level_loc, snow_level);
	glUniform1f(max_grass_level_loc, max_grass_level);
	glUniform1i(wireframe_bool_loc, is_wireframe);
	glUniform1i(is_hidden_loc, is_hidden_crack_fix_enabled);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
	glBindVertexArray(vaoID);
	glDrawElements(GL_PATCHES, 81*4, GL_UNSIGNED_SHORT, NULL);
	glFlush();
}



void keyboard_event_handler (unsigned char key, int x, int y)
{
	if (key == '1') {
		load_texture("HeightMap1.tga", texID[0], GL_TEXTURE0);

	} else if (key == '2') {
		load_texture("HeightMap2.tga", texID[0], GL_TEXTURE0);

	} else if (key == 'w') {
		polygon_mode = (polygon_mode == GL_LINE) ? GL_FILL : GL_LINE;
		is_wireframe ^= true;

	} else if (key == 'r') {
		is_hidden_crack_fix_enabled ^= true;

	} else if (key == 'u') {
		water_level += 0.1;

	} else if (key == 'j') {
		water_level -= 0.1;
		water_level = max(water_level, (float)-0.1);

	} else if (key == 'i') {
		snow_level += 0.1;

	} else if (key == 'k') {
		snow_level -= 0.1;
		snow_level = max(max_grass_level, snow_level); // don't allow snow to go below grass
	}
	glutPostRedisplay();
}



void special_keyboard_event_handler(int key, int x, int y)
{
	int key_incr = 1;

	if (key == GLUT_KEY_UP) {
		z_offset -= key_incr;
	} else if (key == GLUT_KEY_DOWN) {
		z_offset += key_incr;
	} else if (key == GLUT_KEY_LEFT) {
		x_offset -= key_incr;
	} else if (key == GLUT_KEY_RIGHT) {
		x_offset += key_incr;
	}
	glutPostRedisplay();
}



int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Terrain");
	glutInitContextVersion (4, 2);
	glutInitContextProfile ( GLUT_CORE_PROFILE );

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
	glutKeyboardFunc(keyboard_event_handler);
	glutSpecialFunc(special_keyboard_event_handler);
	glutDisplayFunc(display);
	glutMainLoop();
}
