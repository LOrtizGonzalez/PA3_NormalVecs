#include <GL/glew.h>
#include <GL/freeglut.h>
#define GLM_FORCE_CTOR_INIT
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>
#include "shader.h"
#include "shaderprogram.h"

#include <vector>
#define _USE_MATH_DEFINES
#include <math.h> //Allows for M_PI use

/*=================================================================================================
	DOMAIN
=================================================================================================*/

// Window dimensions
const int InitWindowWidth  = 800;
const int InitWindowHeight = 800;
int WindowWidth  = InitWindowWidth;
int WindowHeight = InitWindowHeight;

// Last mouse cursor position
int LastMousePosX = 0;
int LastMousePosY = 0;

// Arrays that track which keys are currently pressed
bool key_states[256];
bool key_special_states[256];
bool mouse_states[8];

// Other parameters
bool draw_wireframe = false;


/*=================================================================================================
	SHADERS & TRANSFORMATIONS
=================================================================================================*/

ShaderProgram PassthroughShader;
ShaderProgram PerspectiveShader;
ShaderProgram PerspectivelightShader;


glm::mat4 PerspProjectionMatrix( 1.0f );
glm::mat4 PerspViewMatrix( 1.0f );
glm::mat4 PerspModelMatrix( 1.0f );

float perspZoom = 1.0f, perspSensitivity = 0.35f;
float perspRotationX = 0.0f, perspRotationY = 0.0f;

/*=================================================================================================
	OBJECTS
=================================================================================================*/

GLuint axis_VAO;
GLuint axis_VBO[2]; //changed from 2

float R = 1.0, r = 0.5; //Use to generate x, y, and z points from TORUS func
int n = 5; //Use to generate x, y, and z points from TORUS func
GLuint torus_VAO; // for TORUS object
GLuint torus_VBO[3]; // for TORUS object; CHANGE VBO FROM 2 TO 4
bool flag = false; //Used to go from flat to smooth

GLuint normal_VAO; //For Flat-Shading -----xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
GLuint normal_VBO[2];
float* flat_vecs;
float* smooth_vecs;

float axis_vertices[] = {
	//x axis
	-1.0f,  0.0f,  0.0f, 1.0f,
	1.0f,  0.0f,  0.0f, 1.0f,
	//y axis
	0.0f, -1.0f,  0.0f, 1.0f,
	0.0f,  1.0f,  0.0f, 1.0f,
	//z axis
	0.0f,  0.0f, -1.0f, 1.0f,
	0.0f,  0.0f,  1.0f, 1.0f
};

float axis_colors[] = {
	//x axis
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	//y axis
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	//z axis
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f
};

float vec_colors[] = {
	1.0f,0.0f,0.0f,1.0f,
	1.0f,0.0f,0.0f,1.0f,
	
};

//--------xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-------- +

float prmtr(int a, int y)
{
	return a * ((2 * M_PI) / y);
}

float* torus_Vertex(float outR,float inr, int n)
{
	std::vector<float> temp;
	int count = 0;

	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			//First Triangle coordinates: x1, y1, z1, 1; x2, y2, z2, 1; x3, y3, z3, 1;
			temp.push_back((outR + inr * cos(prmtr(j, n))) * cos(prmtr(i, n)));
			temp.push_back((outR + inr * cos(prmtr(j, n))) * sin(prmtr(i, n)));
			temp.push_back(inr * sin(prmtr(j, n)));
			temp.push_back(1);

			temp.push_back((outR + inr * cos(prmtr(j, n))) * cos(prmtr(i + 1, n)));
			temp.push_back((outR + inr * cos(prmtr(j, n))) * sin(prmtr(i + 1, n)));
			temp.push_back(inr * sin(prmtr(j, n)));
			temp.push_back(1);

			temp.push_back((outR + inr * cos(prmtr(j + 1, n))) * cos(prmtr(i, n)));
			temp.push_back((outR + inr * cos(prmtr(j + 1, n))) * sin(prmtr(i, n)));
			temp.push_back(inr * sin(prmtr(j + 1, n)));
			temp.push_back(1);
			//Second Triangle
			temp.push_back((outR + inr * cos(prmtr(j + 1, n))) * cos(prmtr(i + 1, n)));
			temp.push_back((outR + inr * cos(prmtr(j + 1, n))) * sin(prmtr(i + 1, n)));
			temp.push_back(inr * sin(prmtr(j + 1, n)));
			temp.push_back(1);

			temp.push_back((outR + inr * cos(prmtr(j + 1, n))) * cos(prmtr(i, n)));
			temp.push_back((outR + inr * cos(prmtr(j + 1, n))) * sin(prmtr(i, n)));
			temp.push_back(inr * sin(prmtr(j + 1, n)));
			temp.push_back(1);

			temp.push_back((outR + inr * cos(prmtr(j, n))) * cos(prmtr(i + 1, n)));
			temp.push_back((outR + inr * cos(prmtr(j, n))) * sin(prmtr(i + 1, n)));
			temp.push_back(inr * sin(prmtr(j, n)));
			temp.push_back(1);

			count += 24;
		}
	}
	float* sample = new float[count];
	for (int i = 0; i < count; i++) {
		sample[i] = temp[i];
	}
	return sample;
}

//ADDED THESE FUNCTIONS FOR THE SHADING
//float* flat_Shading(int no)
//{
//	std::vector<float> temp;
//	int count = 0;
//	
//	for (float i = 0; i < no; ++i) {
//		for (float j = 0; j < no; ++j) {
//			//First Triangle coordinates: x1, y1, z1, 1; x2, y2, z2, 1; x3, y3, z3, 1;
//			temp.push_back((cos(prmtr(j + .5, no))) * cos(prmtr(i + .5, no)));
//			temp.push_back((cos(prmtr(j + .5, no))) * sin(prmtr(i + .5, no)));
//			temp.push_back(sin(prmtr(j + .5, no)));
//			temp.push_back(1);
//
//			temp.push_back((cos(prmtr(j + .5, no))) * cos(prmtr(i + .5, no)));
//			temp.push_back((cos(prmtr(j + .5, no))) * sin(prmtr(i + .5, no)));
//			temp.push_back(sin(prmtr(j + .5, no)));
//			temp.push_back(1);
//
//			temp.push_back((cos(prmtr(j + .5, no))) * cos(prmtr(i + .5, no)));
//			temp.push_back((cos(prmtr(j + .5, no))) * sin(prmtr(i + .5, no))); //----
//			temp.push_back(sin(prmtr(j + .5, no)));
//			temp.push_back(1);
//
//			//Second Triangle
//			temp.push_back((cos(prmtr(j + .5, no))) * cos(prmtr(i + .5, no)));
//			temp.push_back((cos(prmtr(j + .5, no))) * sin(prmtr(i + .5, no)));
//			temp.push_back(sin(prmtr(j + .5, no)));
//			temp.push_back(1);
//
//			temp.push_back((cos(prmtr(j + .5, no))) * cos(prmtr(i + .5, no)));
//			temp.push_back((cos(prmtr(j + .5, no))) * sin(prmtr(i + .5, no)));
//			temp.push_back(sin(prmtr(j + .5, no)));
//			temp.push_back(1);
//
//			temp.push_back((cos(prmtr(j + .5, no))) * cos(prmtr(i + .5, no)));
//			temp.push_back((cos(prmtr(j + .5, no))) * sin(prmtr(i + .5, no)));
//			temp.push_back(sin(prmtr(j + .5, no)));
//			temp.push_back(1);
//
//			count += 24;
//		}
//	}
//	float* sample = new float[count];
//	for (int i = 0; i < count; i++) {
//		sample[i] = temp[i];
//	}
//	return sample;
//}
//
float* smooth_Shading(int n)
{
	std::vector<float> temp;
	int count = 0;

	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			//First Triangle coordinates: x1, y1, z1, 1; x2, y2, z2, 1; x3, y3, z3, 1;
			temp.push_back((cos(prmtr(j , n))) * cos(prmtr(i, n)));
			temp.push_back((cos(prmtr(j, n))) * sin(prmtr(i, n)));
			temp.push_back(sin(prmtr(j, n)));
			temp.push_back(1);

			temp.push_back((cos(prmtr(j, n))) * cos(prmtr(i + 1, n)));
			temp.push_back((cos(prmtr(j, n))) * sin(prmtr(i + 1, n)));
			temp.push_back(sin(prmtr(j, n)));
			temp.push_back(1);

			temp.push_back((cos(prmtr(j + 1, n))) * cos(prmtr(i, n)));
			temp.push_back((cos(prmtr(j + 1, n))) * sin(prmtr(i , n)));
			temp.push_back(sin(prmtr(j + 1, n)));
			temp.push_back(1);

			//Second Triangle
			temp.push_back((cos(prmtr(j + 1, n))) * cos(prmtr(i + 1, n)));
			temp.push_back((cos(prmtr(j + 1, n))) * sin(prmtr(i + 1, n)));
			temp.push_back(sin(prmtr(j + 1, n)));
			temp.push_back(1);

			temp.push_back((cos(prmtr(j + 1, n))) * cos(prmtr(i, n)));
			temp.push_back((cos(prmtr(j + 1, n))) * sin(prmtr(i, n)));
			temp.push_back(sin(prmtr(j + 1, n)));
			temp.push_back(1);

			temp.push_back((cos(prmtr(j, n))) * cos(prmtr(i + 1, n)));
			temp.push_back((cos(prmtr(j, n))) * sin(prmtr(i + 1, n)));
			temp.push_back(sin(prmtr(j + 1, n)));
			temp.push_back(1);

			count += 24;
		}
	}
	float* sample = new float[count];
	for (int i = 0; i < count; i++) {
		sample[i] = temp[i];
	}
	return sample;
}

float* flat_Normals(float outR, float inr, int no)
{
	std::vector<float> temp;
	std::vector<float> v;
	//flat_vecs = &v[0];
	int count = 0;


	for (float i = 0; i < no; ++i) {
		for (float j = 0; j < no; ++j) {
			//First Triangle coordinates: x1, y1, z1, 1; x2, y2, z2, 1; x3, y3, z3, 1;
			glm::vec3 a((outR + inr * cos(prmtr(j, n))) * cos(prmtr(i, n)),
				(outR + inr * cos(prmtr(j, n))) * sin(prmtr(i, n)), //
				inr * sin(prmtr(j, n)));
			
			glm::vec3 b((outR + inr * cos(prmtr(j, n))) * cos(prmtr(i + 1, n)), //
				(outR + inr * cos(prmtr(j, n))) * sin(prmtr(i + 1, n)),
				inr * sin(prmtr(j, n)));

			glm::vec3 c((outR + inr * cos(prmtr(j + 1, n))) * cos(prmtr(i, n)),
				(outR + inr * cos(prmtr(j + 1, n))) * sin(prmtr(i, n)),
				inr * sin(prmtr(j + 1, n)));
			//SECOND TRIANGLE 
			glm::vec3 d((outR + inr * cos(prmtr(j + 1, n))) * cos(prmtr(i + 1, n)),
				(outR + inr * cos(prmtr(j + 1, n))) * sin(prmtr(i + 1, n)),
				inr * sin(prmtr(j + 1, n)));

			glm::vec3 e((outR + inr * cos(prmtr(j + 1, n))) * cos(prmtr(i, n)),
				(outR + inr * cos(prmtr(j + 1, n))) * sin(prmtr(i, n)),
				inr * sin(prmtr(j + 1, n)));

			glm::vec3 f((outR + inr * cos(prmtr(j, n))) * cos(prmtr(i + 1, n)),
				(outR + inr * cos(prmtr(j, n))) * sin(prmtr(i + 1, n)),
				inr * sin(prmtr(j, n)));

			glm::vec3 BA = (a - b);
			glm::vec3 BC = (c - b);
			glm::vec3 ED = (d - e);;
			glm::vec3 EF = (f - e);;

			BA = glm::normalize(BA);
			BC = glm::normalize(BC);
			ED = glm::normalize(ED);
			EF = glm::normalize(EF);

			glm::vec3 ASD = glm::cross(BA, BC);
			glm::vec3 FDS = glm::cross(ED, EF);

			glm::vec4 norm1 = glm::vec4(ASD, 1.0);
			glm::vec4 norm2 = glm::vec4(FDS, 1.0);
			//Norm for Triangle 1
			temp.push_back(-1 * norm1[0]); temp.push_back(-1 * norm1[1]); temp.push_back(-1 * norm1[2]); temp.push_back(-1 * norm1[3]);
			temp.push_back(-1 * norm1[0]); temp.push_back(-1 * norm1[1]); temp.push_back(-1 * norm1[2]); temp.push_back(-1 * norm1[3]);
			temp.push_back(-1 * norm1[0]); temp.push_back(-1 * norm1[1]); temp.push_back(-1 * norm1[2]); temp.push_back(-1 * norm1[3]);
			//Norm for Triangle 2
			temp.push_back(-1 * norm2[0]); temp.push_back(-1 * norm2[1]); temp.push_back(-1 * norm2[2]); temp.push_back(-1 * norm2[3]);
			temp.push_back(-1 * norm2[0]); temp.push_back(-1 * norm2[1]); temp.push_back(-1 * norm2[2]); temp.push_back(-1 * norm2[3]);
			temp.push_back(-1 * norm2[0]); temp.push_back(-1 * norm2[1]); temp.push_back(-1 * norm2[2]); temp.push_back(-1 * norm2[3]);
			//for v vector
			v.push_back((a[0] + b[0] + c[0]) / 3); v.push_back((a[1] + b[1] + c[1]) / 3); v.push_back((a[2] + b[2] + c[2]) / 3); v.push_back(1);
			v.push_back(-1 * norm1[0]); v.push_back(-1 * norm1[1]); v.push_back(-1 * norm1[2]); v.push_back(-1 * norm1[3]);
			v.push_back((d[0] + e[0] + f[0]) / 3); v.push_back((d[1] + e[1] + f[1]) / 3); v.push_back((d[2] + e[2] + f[2]) / 3); v.push_back(1);
			v.push_back(-1 * norm2[0]); v.push_back(-1 * norm2[1]); v.push_back(-1 * norm2[2]); v.push_back(-1 * norm2[3]);

			count += 24;
			
		}
	}

	
	float* sample = new float[count];
	for (int i = 0; i < count; i++) {
		sample[i] = 1*temp[i];
	}

	return sample;
}

float* returnNormalValues(float outR, float inr, int no)
{
	std::vector<float> v;
	int countNorms = 0;

	for (float i = 0; i < no; ++i) {
		for (float j = 0; j < no; ++j) {
			//First Triangle coordinates: x1, y1, z1, 1; x2, y2, z2, 1; x3, y3, z3, 1;
			glm::vec3 a((outR + inr * cos(prmtr(j, n))) * cos(prmtr(i, n)),
				(outR + inr * cos(prmtr(j, n))) * sin(prmtr(i, n)), //
				inr * sin(prmtr(j, n)));

			glm::vec3 b((outR + inr * cos(prmtr(j, n))) * cos(prmtr(i + 1, n)), //
				(outR + inr * cos(prmtr(j, n))) * sin(prmtr(i + 1, n)),
				inr * sin(prmtr(j, n)));

			glm::vec3 c((outR + inr * cos(prmtr(j + 1, n))) * cos(prmtr(i, n)),
				(outR + inr * cos(prmtr(j + 1, n))) * sin(prmtr(i, n)),
				inr * sin(prmtr(j + 1, n)));
			//SECOND TRIANGLE 
			glm::vec3 d((outR + inr * cos(prmtr(j + 1, n))) * cos(prmtr(i + 1, n)),
				(outR + inr * cos(prmtr(j + 1, n))) * sin(prmtr(i + 1, n)),
				inr * sin(prmtr(j + 1, n)));

			glm::vec3 e((outR + inr * cos(prmtr(j + 1, n))) * cos(prmtr(i, n)),
				(outR + inr * cos(prmtr(j + 1, n))) * sin(prmtr(i, n)),
				inr * sin(prmtr(j + 1, n)));

			glm::vec3 f((outR + inr * cos(prmtr(j, n))) * cos(prmtr(i + 1, n)),
				(outR + inr * cos(prmtr(j, n))) * sin(prmtr(i + 1, n)),
				inr * sin(prmtr(j, n)));

			glm::vec3 BA = (a - b);
			glm::vec3 BC = (c - b);
			glm::vec3 ED = (d - e);
			glm::vec3 EF = (f - e);

			glm::vec3 ASD = glm::cross(BA, BC);
			glm::vec3 FDS = glm::cross(ED, EF);

			ASD = glm::normalize(ASD);
			FDS = glm::normalize(FDS);

			glm::vec4 norm1 = glm::vec4(ASD, 1.0);
			glm::vec4 norm2 = glm::vec4(FDS, 1.0);

			//for v vector
			glm::vec3 midpoint1((a[0] + b[0] + c[0]) / 3, (a[1] + b[1] + c[1]) / 3, (a[2] + b[2] + c[2]) / 3);
			glm::vec3 midpoint2((d[0] + e[0] + f[0]) / 3, (d[1] + e[1] + f[1]) / 3, (d[2] + e[2] + f[2]) / 3);

			midpoint1 = midpoint1 - ASD;
			midpoint2 = midpoint2 - FDS;
			
			norm1[0] += midpoint1[0]; norm1[1] += midpoint1[1]; norm1[2] += midpoint1[2];
			norm2[0] += midpoint2[0]; norm2[1] += midpoint2[1]; norm2[2] += midpoint2[2];
			
			v.push_back(midpoint1[0]); v.push_back(midpoint1[1]); v.push_back(midpoint1[2]); v.push_back(1);
			v.push_back(norm1[0]); v.push_back(norm1[1]); v.push_back(norm1[2]); v.push_back(norm1[3]);
			v.push_back(midpoint2[0]); v.push_back(midpoint2[1]); v.push_back(midpoint2[2]); v.push_back(1);
			v.push_back(norm2[0]); v.push_back(norm2[1]); v.push_back(norm2[2]); v.push_back(norm2[3]);
			
			countNorms += 16;
		}
	}
	float* sample = new float[countNorms];
	for (int i = 0; i < countNorms; i++) {
		sample[i] = 1 * v[i];
	}

	return sample;
}

float* returnSmoothValues(float R, float r, int n) {
	int countNorms = 0;
	std::vector<float> v;

	for (float i = 0; i < n; ++i) {
		for (float j = 0; j < n; ++j) {
			//First Triangle coordinates: x1, y1, z1, 1; x2, y2, z2, 1; x3, y3, z3, 1;
			glm::vec3 a((R + r * cos(prmtr(j, n))) * cos(prmtr(i, n)),
				(R + r * cos(prmtr(j, n))) * sin(prmtr(i, n)), //
				r * sin(prmtr(j, n)));

			glm::vec3 b((R + r * cos(prmtr(j, n))) * cos(prmtr(i + 1, n)), //
				(R + r * cos(prmtr(j, n))) * sin(prmtr(i + 1, n)),
				r * sin(prmtr(j, n)));

			glm::vec3 c((R + r * cos(prmtr(j + 1, n))) * cos(prmtr(i, n)),
				(R + r * cos(prmtr(j + 1, n))) * sin(prmtr(i, n)),
				r * sin(prmtr(j + 1, n)));

			//SECOND TRIANGLE 
			glm::vec3 d((R + r * cos(prmtr(j + 1, n))) * cos(prmtr(i + 1, n)),
				(R + r * cos(prmtr(j + 1, n))) * sin(prmtr(i + 1, n)),
				r * sin(prmtr(j + 1, n)));

			glm::vec3 e((R + r * cos(prmtr(j + 1, n))) * cos(prmtr(i, n)),
				(R + r * cos(prmtr(j + 1, n))) * sin(prmtr(i, n)),
				r * sin(prmtr(j + 1, n)));

			glm::vec3 f((R + r * cos(prmtr(j, n))) * cos(prmtr(i + 1, n)),
				(R + r * cos(prmtr(j, n))) * sin(prmtr(i + 1, n)),
				r * sin(prmtr(j, n)));

			glm::vec3 mid(R + r * (cos(prmtr(j, n))),
				-r,
				r * (sin(j) - 1));

			a = glm::normalize(a);
			b = glm::normalize(b);
			c = glm::normalize(c);
			d = glm::normalize(d);
			e = glm::normalize(e);
			f = glm::normalize(f);
			a -= mid;
			b -= mid;
			c -= mid;
			d -= mid;
			e -= mid;
			f -= mid;

			glm::vec4 norm1 = glm::vec4(a, 1.0);
			glm::vec4 norm2 = glm::vec4(b, 1.0);
			glm::vec4 norm3 = glm::vec4(c, 1.0);
			glm::vec4 norm4 = glm::vec4(d, 1.0);
			glm::vec4 norm5 = glm::vec4(e, 1.0);
			glm::vec4 norm6 = glm::vec4(f, 1.0);

			//for v vector
			//glm::vec3 midpoint1((a[0] + b[0] + c[0]) / 3, (a[1] + b[1] + c[1]) / 3, (a[2] + b[2] + c[2]) / 3);
			//glm::vec3 midpoint2((d[0] + e[0] + f[0]) / 3, (d[1] + e[1] + f[1]) / 3, (d[2] + e[2] + f[2]) / 3);

			//norm1[0] += midpoint1[0]; norm1[1] += midpoint1[1]; norm1[2] += midpoint1[2];
			//norm2[0] += midpoint2[0]; norm2[1] += midpoint2[1]; norm2[2] += midpoint2[2];

			v.push_back(norm1[0]); v.push_back(norm1[1]); v.push_back(norm1[2]); v.push_back(norm1[3]);
			v.push_back(norm2[0]); v.push_back(norm2[1]); v.push_back(norm2[2]); v.push_back(norm2[3]);
			v.push_back(norm3[0]); v.push_back(norm3[1]); v.push_back(norm3[2]); v.push_back(norm3[3]);
			v.push_back(norm4[0]); v.push_back(norm4[1]); v.push_back(norm4[2]); v.push_back(norm4[3]);
			v.push_back(norm5[0]); v.push_back(norm5[1]); v.push_back(norm5[2]); v.push_back(norm5[3]);
			v.push_back(norm6[0]); v.push_back(norm6[1]); v.push_back(norm6[2]); v.push_back(norm6[3]);

			countNorms += 24;
		}
	}
	float* sample = new float[countNorms];
	for (int i = 0; i < countNorms; i++) {
		sample[i] = 1 * v[i];
	}

	return sample;
}


//---------xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx--------- +

/*=================================================================================================
	HELPER FUNCTIONS
=================================================================================================*/

void window_to_scene( int wx, int wy, float& sx, float& sy )
{
	sx = ( 2.0f * (float)wx / WindowWidth ) - 1.0f;
	sy = 1.0f - ( 2.0f * (float)wy / WindowHeight );
}

/*=================================================================================================
	SHADERS
=================================================================================================*/

void CreateTransformationMatrices( void )
{
	// PROJECTION MATRIX
	PerspProjectionMatrix = glm::perspective<float>( glm::radians( 60.0f ), (float)WindowWidth / (float)WindowHeight, 0.01f, 1000.0f );

	// VIEW MATRIX
	glm::vec3 eye   ( 0.0, 0.0, 2.0 );
	glm::vec3 center( 0.0, 0.0, 0.0 );
	glm::vec3 up    ( 0.0, 1.0, 0.0 );

	PerspViewMatrix = glm::lookAt( eye, center, up );

	// MODEL MATRIX
	PerspModelMatrix = glm::mat4( 1.0 );
	PerspModelMatrix = glm::rotate( PerspModelMatrix, glm::radians( perspRotationX ), glm::vec3( 1.0, 0.0, 0.0 ) );
	PerspModelMatrix = glm::rotate( PerspModelMatrix, glm::radians( perspRotationY ), glm::vec3( 0.0, 1.0, 0.0 ) );
	PerspModelMatrix = glm::scale( PerspModelMatrix, glm::vec3( perspZoom ) );
}

void CreateShaders( void )
{
	// Renders without any transformations
	PassthroughShader.Create( "./shaders/simple.vert", "./shaders/simple.frag" );

	// Renders using perspective projection
	PerspectiveShader.Create( "./shaders/persp.vert", "./shaders/persp.frag" );

	//perpslight.frag/vert
	PerspectivelightShader.Create("./shaders/persplight.vert", "./shaders/persplight.frag"); //New Shader for Torus
}

/*=================================================================================================
	BUFFERS
=================================================================================================*/

void CreateAxisBuffers( void )
{
	glGenVertexArrays( 1, &axis_VAO );
	glBindVertexArray( axis_VAO );

	glGenBuffers( 2, &axis_VBO[0] );//changed from 2

	glBindBuffer( GL_ARRAY_BUFFER, axis_VBO[0] );
	glBufferData( GL_ARRAY_BUFFER, sizeof( axis_vertices ), axis_vertices, GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void*)0 );
	glEnableVertexAttribArray( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, axis_VBO[1] );
	glBufferData( GL_ARRAY_BUFFER, sizeof( axis_colors ), axis_colors, GL_STATIC_DRAW );
	glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void*)0 );
	glEnableVertexAttribArray( 1 );

	glBindVertexArray( 0 );
	//===xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

	/*glGenVertexArrays(1, &normal_VAO);
	glBindVertexArray(normal_VAO);

	glGenBuffers(2, &normal_VBO[0]);

	

	glBindBuffer(GL_ARRAY_BUFFER, normal_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, n * n * 24 * sizeof(float), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, normal_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, n * n * 24 * sizeof(float), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);*/
}

void CreateTorusBuffers(void)
{
	glGenVertexArrays(1, &torus_VAO);
	glBindVertexArray(torus_VAO);

	glGenBuffers(3, &torus_VBO[0]);

	float* torus = torus_Vertex(R, r, n);

	glBindBuffer(GL_ARRAY_BUFFER, torus_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, n * n * 24 * sizeof(float), torus, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, torus_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, n * n * 24 * sizeof(float), torus, GL_STATIC_DRAW); 
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	// ADDED THIS FOR FLAT SHADING XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	float* norms;
	if (flag == false)
		norms = flat_Normals(R, r, n);
	else
		norms = smooth_Shading( n);
	
	glBindBuffer(GL_ARRAY_BUFFER, torus_VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, n * n * 24 * sizeof(float), norms, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);
	

	glBindVertexArray(0);
	delete[] torus; //ADDED DELETES TO CLEAR MEMORY
	delete[] norms;
}

void CreateNormalVecs(void)
{/*
	glGenVertexArrays(1, &normal_VAO);
	glBindVertexArray(normal_VAO);

	glGenBuffers(2, &normal_VBO[0]);
	
	float* normValues = returnNormalValues(R,r,n);

	glBindBuffer(GL_ARRAY_BUFFER, normal_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, n * n * 16 * sizeof(float), normValues, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, normal_VBO[1]); //Needs to be for color of object
	glBufferData(GL_ARRAY_BUFFER,n*n*16*sizeof(float), normValues, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	delete[] normValues;*/
}

void CreateSmoothVecs(void) {

	glGenVertexArrays(1, &normal_VAO);
	glBindVertexArray(normal_VAO);

	glGenBuffers(2, &normal_VBO[0]);

	float* normValues = returnSmoothValues(R, r, n);

	glBindBuffer(GL_ARRAY_BUFFER, normal_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, n * n * 24 * sizeof(float), normValues, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, normal_VBO[1]); //Needs to be for color of object
	glBufferData(GL_ARRAY_BUFFER, n * n * 24 * sizeof(float), normValues, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	delete[] normValues;
}

/*=================================================================================================
	CALLBACKS
=================================================================================================*/

//-----------------------------------------------------------------------------
// CALLBACK DOCUMENTATION
// https://www.opengl.org/resources/libraries/glut/spec3/node45.html
// http://freeglut.sourceforge.net/docs/api.php#WindowCallback
//-----------------------------------------------------------------------------

void idle_func()
{
	//uncomment below to repeatedly draw new frames
	glutPostRedisplay();
}

void reshape_func( int width, int height )
{
	WindowWidth  = width;
	WindowHeight = height;

	glViewport( 0, 0, width, height );
	glutPostRedisplay();
}

void keyboard_func( unsigned char key, int x, int y )
{
	key_states[ key ] = true;

	switch( key )
	{
		case 'f':
		{
			draw_wireframe = !draw_wireframe;
			if( draw_wireframe == true )
				std::cout << "Wireframes on.\n";
			else
				std::cout << "Wireframes off.\n";
			break;
		}
		case 'q':
		{
			if (n < 75)
				n++;
			CreateTorusBuffers();
			CreateNormalVecs();
			glutPostRedisplay();
			break;
		}
		case 'a':
		{
			if (n > 3) {
				n--;
				CreateTorusBuffers();
				CreateNormalVecs();
				glutPostRedisplay();
			}
			break;
		}
		case'w':
		{
			if (r <= 1.5)
				r = r + 0.01;
			CreateTorusBuffers();
			CreateNormalVecs();
			glutPostRedisplay();
			break;
		}
		case's':
		{
			if (r > 0.1) 
				r = r - 0.01;
			CreateTorusBuffers();
			CreateNormalVecs();
			glutPostRedisplay();
			break;
		}
		case'e':
		{
			if (R <= 5)
				R = R + 0.01;
			CreateTorusBuffers();
			CreateNormalVecs();
			glutPostRedisplay();
			break;
		}
		case'd':
		{
			if (R > 0.5)
				R = R - 0.01;
			CreateTorusBuffers();
			CreateNormalVecs();
			glutPostRedisplay();
			break;
		}
		case'z':
		{
			flag = false;
			CreateTorusBuffers();
			glutPostRedisplay();
			break;
		}
		case'x':
		{
			flag = true;
			CreateTorusBuffers();
			glutPostRedisplay();
			break;
		}

		// Exit on escape key press
		case '\x1B':
		{
			exit( EXIT_SUCCESS );
			break;
		}
	}
	glutPostRedisplay();
}

void key_released( unsigned char key, int x, int y )
{
	key_states[ key ] = false;
}

void key_special_pressed( int key, int x, int y )
{
	key_special_states[ key ] = true;
}

void key_special_released( int key, int x, int y )
{
	key_special_states[ key ] = false;
}

void mouse_func( int button, int state, int x, int y )
{
	// Key 0: left button
	// Key 1: middle button
	// Key 2: right button
	// Key 3: scroll up
	// Key 4: scroll down

	if( x < 0 || x > WindowWidth || y < 0 || y > WindowHeight )
		return;

	float px, py;
	window_to_scene( x, y, px, py );

	if( button == 3 )
	{
		perspZoom += 0.03f;
	}
	else if( button == 4 )
	{
		if( perspZoom - 0.03f > 0.0f )
			perspZoom -= 0.03f;
	}

	mouse_states[ button ] = ( state == GLUT_DOWN );

	LastMousePosX = x;
	LastMousePosY = y;
}

void passive_motion_func( int x, int y )
{
	if( x < 0 || x > WindowWidth || y < 0 || y > WindowHeight )
		return;

	float px, py;
	window_to_scene( x, y, px, py );

	LastMousePosX = x;
	LastMousePosY = y;
}

void active_motion_func( int x, int y )
{
	if( x < 0 || x > WindowWidth || y < 0 || y > WindowHeight )
		return;

	float px, py;
	window_to_scene( x, y, px, py );

	if( mouse_states[0] == true )
	{
		perspRotationY += ( x - LastMousePosX ) * perspSensitivity;
		perspRotationX += ( y - LastMousePosY ) * perspSensitivity;
	}

	LastMousePosX = x;
	LastMousePosY = y;
}

/*=================================================================================================
	RENDERING
=================================================================================================*/

void display_func( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	CreateTransformationMatrices();

	PerspectiveShader.Use();
	PerspectiveShader.SetUniform( "projectionMatrix", glm::value_ptr( PerspProjectionMatrix ), 4, GL_FALSE, 1 );
	PerspectiveShader.SetUniform( "viewMatrix", glm::value_ptr( PerspViewMatrix ), 4, GL_FALSE, 1 );
	PerspectiveShader.SetUniform( "modelMatrix", glm::value_ptr( PerspModelMatrix ), 4, GL_FALSE, 1 );

	if( draw_wireframe == true )
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	glBindVertexArray( axis_VAO );
	glDrawArrays( GL_LINES, 0, 6 );

	glBindVertexArray( 0 );

	//--------------------------------------------------------+
	PerspectivelightShader.Use();
	PerspectivelightShader.SetUniform("projectionMatrix", glm::value_ptr(PerspProjectionMatrix), 4, GL_FALSE, 1);
	PerspectivelightShader.SetUniform("viewMatrix", glm::value_ptr(PerspViewMatrix), 4, GL_FALSE, 1);
	PerspectivelightShader.SetUniform("modelMatrix", glm::value_ptr(PerspModelMatrix), 4, GL_FALSE, 1);

	glBindVertexArray( torus_VAO ); //Added for TORUS
	glDrawArrays(GL_TRIANGLES, 0, n * n * 6); //Added for TORUS
	glBindVertexArray(0);
	//--------------------------------------------------------+
	glBindVertexArray(normal_VAO);
	glDrawArrays(GL_LINES, 0, n * n * 24);
	glBindVertexArray(0);

	if( draw_wireframe == true )
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	glutSwapBuffers();
}

/*=================================================================================================
	INIT
=================================================================================================*/

void init( void )
{
	// Print some info
	std::cout << "Vendor:         " << glGetString( GL_VENDOR   ) << "\n";
	std::cout << "Renderer:       " << glGetString( GL_RENDERER ) << "\n";
	std::cout << "OpenGL Version: " << glGetString( GL_VERSION  ) << "\n";
	std::cout << "GLSL Version:   " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n\n";

	// Set OpenGL settings
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ); // background color
	glEnable( GL_DEPTH_TEST ); // enable depth test
	glEnable( GL_CULL_FACE ); // enable back-face culling

	// Create shaders
	CreateShaders();

	// Create buffers
	CreateAxisBuffers();
	CreateTorusBuffers(); //From the torus buffers
	CreateNormalVecs();
	CreateSmoothVecs();

	std::cout << "Finished initializing...\n\n";
}

/*=================================================================================================
	MAIN
=================================================================================================*/

int main( int argc, char** argv )
{
	glutInit( &argc, argv );

	glutInitWindowPosition( 100, 100 );
	glutInitWindowSize( InitWindowWidth, InitWindowHeight );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );

	glutCreateWindow( "CSE-170 Computer Graphics" );

	// Initialize GLEW
	GLenum ret = glewInit();
	if( ret != GLEW_OK ) {
		std::cerr << "GLEW initialization error." << std::endl;
		glewGetErrorString( ret );
		return -1;
	}

	glutDisplayFunc( display_func );
	glutIdleFunc( idle_func );
	glutReshapeFunc( reshape_func );
	glutKeyboardFunc( keyboard_func );
	glutKeyboardUpFunc( key_released );
	glutSpecialFunc( key_special_pressed );
	glutSpecialUpFunc( key_special_released );
	glutMouseFunc( mouse_func );
	glutMotionFunc( active_motion_func );
	glutPassiveMotionFunc( passive_motion_func );

	init();

	glutMainLoop();

	return EXIT_SUCCESS;
}






/*float* normals;
if (flag == false) {
	normals = flat_Shading(n);
}
else
normals = smooth_Shading(n);

glBindBuffer(GL_ARRAY_BUFFER, torus_VBO[2]);
glBufferData(GL_ARRAY_BUFFER, n * n * 24 * sizeof(float), normals, GL_STATIC_DRAW);
glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
glEnableVertexAttribArray(2);

glBindBuffer(GL_ARRAY_BUFFER, torus_VBO[3]);
glBufferData(GL_ARRAY_BUFFER, n * n * 24 * sizeof(float), normals, GL_STATIC_DRAW);
glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
glEnableVertexAttribArray(3);*/

/*
for flat shading, make sure to calculate and push 6 points so the the shading works on all the triangles. the 
way i calculated as of now, only some of the triangles are shaded so it looks weird

for the vectors, push two points, but might have to create a new buffer for them since they are a different size than 
the arays/points used in the torus_buffer
*/