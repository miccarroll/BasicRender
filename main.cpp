#include <iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include <string>
#include <cmath>
#include <limits>
#include <vector>

#include"shaderClass.h"
#include"VAO.h"
#include"VBO.h"

using namespace std;


// makes this either 1,2 or 3. nothing else!
int resolution = 3;

GLfloat points[5000000];

float trueres     = pow(10, resolution);
float invtrueres  = 1/trueres;
float trueresD100 = trueres/100.0f;

class Color;
class Vector3;
class Vector2;
class Vertex;
class Edge;

void drawPixel(Vector2* point, Color* rgb);
void drawPixel(Vertex* v);

class Vector3 {
	public:
		float x;
		float y;
		float z;
		Vector3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
			x = _x;
			y = _y;
			z = _z;
		}
	};

class Vector2 {
	public:
	float x;
	float y;

	Vector2(float _x = 0.0f, float _y = 0.0f) {
		x = _x;
		y = _y;
	}
};

class Color{
	public:
		float r;
		float g;
		float b;
		Color(float _r = 0.0f, float _g = 0.0f, float _b = 0.0f) {
			r = _r;
			b = _g;
			g = _b;
		}
};

class Vertex: public Vector3, public Color{
	public:
	float lx;
	float ly;
	float lz;

	Vertex (float _x, float _y, float _z,float _r, float _g, float _b){
		x = _x;
		y = _y;
		z = _z;
		lx = 0;
		ly = 0;
		lz = 0;
		r = _r;
		g = _g;
		b = _b;
	}
	Vertex(float _x, float _y, float _z,Color rgb) {
		x = _x;
		y = _y;
		z = _z;
		lx = 0;
		ly = 0;
		lz = 0;
		r = rgb.r;
		g = rgb.g;
		b = rgb.b;
	}

	bool operator =(Vertex const &_v) const{
		if(x == _v.x && y == _v.y){
			return true;
		}else{
			return false;
		}
	}

	void Vis2D(float _x = -1.0f, float _y = -1.0f){
		if(_x != -1.0f || _y != -1.0f){
			x = _x;
			y = _y;
		}
		drawPixel(this);
	} 
	
	void setLocal(Vector3 origin){
		lx = x - origin.x;
		ly = y - origin.y;
		lz = z - origin.z;
	}

	void setGlobal(Vector3 origin){
		x = lx + origin.x;
		y = ly + origin.y;
		z = lz + origin.z;
	}
};

class Edge: public Color {
	public:
	Vertex* u;
	Vertex* v;

	Edge(Vertex* _u, Vertex* _v, float _r, float _g, float _b) {
		u = _u;
		v = _v;
		r = _r;
		g = _g;
		b = _b;
	}
	
	Edge (Vertex* _u, Vertex* _v,Color rgb){
		u = _u;
		v = _v;
		r = rgb.r;
		g = rgb.g;
		b = rgb.b;
	}

	void Vis2D(){
		float scale = pow(10, resolution - 2);
		float v_x = v->x * scale;
		float v_y = v->y * scale;
		float u_x = u->x * scale;
		float u_y = u->y * scale;
		
		float Dx = v_x - u_x;
		float Dy = v_y - u_y;

		float Fx = floor(v_x);
		float Fy = floor(v_y);

		float steps = abs(Dx) > abs(Dy) ? abs(Dx) : abs(Dy);

		if (steps == 0) {
			steps = .000001f;
		}

		float xinc = Dx / steps;
		float yinc = Dy / steps;

		Vector2 pixel(u_x, u_y);
		Color pixelCol(r,g,b);
		for(int i = 0; i <= steps; i++){
			Vector2* newPix = new Vector2(pixel.x / scale, pixel.y / scale);
			drawPixel(newPix, &pixelCol);
			delete newPix;
			pixel.x += xinc;
			pixel.y += yinc;	
		}
	}
};

class Face {
	public:
	std::vector<Edge*> edges;
	std::vector<Vertex*> verts;
	Vector3 origin;
	Vector3 rotation;
	
	Face(std::vector<Edge*> _edges){
		edges = _edges;
		float x_sum = 0.0f;
		float y_sum = 0.0f;
		float z_sum = 0.0f;
		
		for(Edge* edge : edges){
			Vertex* e_u = edge->u;
			Vertex* e_v = edge->v;
			

			if(std::find(verts.begin(), verts.end(), edge->u) == verts.end()){
				verts.push_back(edge->u);
				x_sum += edge->u->x;
				y_sum += edge->u->y;
				z_sum += edge->u->z;
			}

			if (std::find(verts.begin(), verts.end(), edge->v) == verts.end()) {
				verts.push_back(edge->v);
				x_sum += edge->v->x;
				y_sum += edge->v->y;
				z_sum += edge->v->z;
			}
		}

		int verts_count = verts.size();
		origin = Vector3(x_sum / verts_count, y_sum / verts_count, z_sum / verts_count);
		rotation = Vector3();

		for(Vertex* vertex : verts){
			vertex->setLocal(origin);
		}
	}

	void Vis2D(){
		for(Edge* edge : edges){
			edge->Vis2D();
		}
	}

	void rotate2D(float radians){
		rotation.z += radians;
		//rotation.z = fmod(rotation.z, 2.0f);

		float ix = cos(rotation.z);
		float iy = -sin(rotation.z);
		float jx = sin(rotation.z);
		float jy = cos(rotation.z);

		for (Vertex* vert : verts) {
			vert->x = (ix * vert->lx + jx * vert->ly) + origin.x;
			vert->y = (iy * vert->lx + jy * vert->ly) + origin.y;
		}
	}

};

void drawPixel(Vector2* point, Color* rgb) {
	if(point->x < 0.0f || point->y < 0.0f || point->x > 100.0f || point->y > 100.0f){
		return;
	}
	
	float _x = round(point->x * trueresD100) / trueresD100;
	float _y = round(point->y * trueresD100) / trueresD100;
	
	int ix = int((_x * trueresD100 + _y * trueres * trueresD100)) * 5;

	points[ix + 2] = GLfloat(rgb->r);
	points[ix + 3] = GLfloat(rgb->g);
	points[ix + 4] = GLfloat(rgb->b);
}
void drawPixel(Vertex* v) {

	if (v->x < 0 || v->y < 0 || v->x > 100.0f || v->y > 100.0f) {
		return;
	}

	if (v->x > 100) {
		v->x = 100;
	}

	if (v->y > 100) {
		v->y = 100;
	}

	float _x = round(v->x * trueresD100) / trueresD100;
	float _y = round(v->y * trueresD100) / trueresD100;

	int ix = int((_x * trueresD100 + _y * trueres * trueresD100)) * 5;

	points[ix + 2] = GLfloat(v->r);
	points[ix + 3] = GLfloat(v->g);
	points[ix + 4] = GLfloat(v->b);
}

float d2r (float deg){
	float pi = 3.14159265359f;
	return(deg * (pi /180));
}

int main() {

	// Initialize GLFW
	glfwInit();

	// START WINDOW INIT--------------------------------------------

		// Tell GLFW what version of OpenGL we are using 
		// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



	// Create a GLFWwindow object of 800 by 800 pixels, naming it "YoutubeOpenGL"
	GLFWwindow* window = glfwCreateWindow(1000, 1000, "OpenGLTesting", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, 1000, 1000);


	//setting V-Sync
	glfwSwapInterval(2);

	// END WINDOW INIT--------------------------------------------

	Shader shaderProgram("default.vert", "default.frag");

	// Generates Vertex Array Object and binds it

	for (int i = 0; i < trueres; i++) {
		for (int o = 0; o < trueres; o++) {
			int index_x = (o * trueres + i) * 5;
			int index_y = index_x + 1;

			points[index_x] = GLfloat(i / (trueres/2) - 1.0f + invtrueres);
			points[index_y] = GLfloat(o / (trueres/2) - 1.0f + invtrueres);
		}
	}
	/*
  for (int i = 0; i < 495; i += 5) {
		cout << i / 5 << " : " << points[i + 0] << " , " << points[i + 1] << " , " << points[i + 2] << " , " << points[i + 3] << " , " << points[i + 4] << endl;
	}
	*/

	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(pow(10,3-resolution));

	int state = 0;
	float r_value = 0;

  Color drawColor(1,1,1);

	Vertex* Varray [4] = {
		new Vertex(40.0f, 40.0f, 0.0f, 1,1,1),
		new Vertex(60.0f, 40.0f, 0.0f, 1,1,1),
		new Vertex(60.0f, 60.0f, 0.0f, 1,1,1),
		new Vertex(40.0f, 60.0f, 0.0f, 1,1,1),
	};

	vector<Edge*> Earray {
		new Edge(Varray[0],Varray[1],drawColor),
		new Edge(Varray[1],Varray[2],drawColor),
		new Edge(Varray[2],Varray[3],drawColor),
		new Edge(Varray[3],Varray[0],drawColor),
	};

	Face square(Earray);


	// Main while loop
	while (!glfwWindowShouldClose(window)) {
		if (state == 0) {
			if (r_value < 1) {
				r_value += .01f;
			} else {
				r_value -= .01f;
				state = 1;
			}
		} else {
			if (r_value > 0) {
				r_value -= .01f;
			} else {
				r_value += .01f;
				state = 0;
			}
		}

		for (int i = 0; i < trueres; i++) {
			for (int o = 0; o < trueres; o++) {
				int index_x = (i * trueres + o) * 5;
				int index_r = index_x + 2;
				int index_g = index_x + 3;
				int index_b = index_x + 4;

				points[index_r] = GLfloat(i / trueres);
				points[index_g] = GLfloat(o / trueres);
				points[index_b] = GLfloat(r_value);
			}
		}

		square.rotate2D(.005f);

		/*
		for (Edge* e : Earray) {
			e->Vis2D();
		}
		*/
		for (Vertex* v : Varray) {
			v->Vis2D();
		}
		
		square.Vis2D();

		VAO VAO1;
		VAO1.Bind();
		VBO VBO1(points, sizeof(points));
		VAO1.LinkAttrib(VBO1, 0, 2, GL_FLOAT, 5 * sizeof(float), (void*)0);
		VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 5 * sizeof(float), (void*)(2 * sizeof(float)));

		// Clean the back buffer and assign the new color to it
		//glClearColor(r_value, 0.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);
	

		shaderProgram.Activate();

		glDrawArrays(GL_POINTS, 0, pow(trueres,2));
		
		VBO1.Unbind();
		VBO1.Delete();
		VAO1.Unbind();
		VAO1.Delete();

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);

		// Take care of all GLFW events
		glfwPollEvents();
	}



	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}