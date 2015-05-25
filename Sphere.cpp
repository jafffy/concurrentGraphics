#include <vector>

#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Sphere.h"

#define GLSL(src) "#version 330 core\n" #src

Sphere::Sphere()
{
	// construct sphere
	const unsigned stack = 32, slice = 32;

	for (unsigned i = 0; i <= stack; ++i) {
		double rho[2], r[2], x[2], y[2], z[2];
		double theta;
		double v[2], u;

		rho[0] = (90.0 - (   i    * 180.0 / stack)) / 180.0 * M_PI;
		rho[1] = (90.0 - ((i + 1) * 180.0 / stack)) / 180.0 * M_PI;

		y[0] = sin(rho[0]); r[0] = cos(rho[0]);
		y[1] = sin(rho[1]); r[1] = cos(rho[1]);

		v[0] = i / (double)stack; v[1] = (i + 1) / (double)stack;

		for (unsigned j = 0; j <= slice; ++j) {
			theta	= j / (double)slice * M_PI * 2.0;
			u		= j / (double)slice;

			x[0] = sin(theta) * r[0];	z[0] = cos(theta) * r[0];
			x[1] = sin(theta) * r[1];	z[1] = cos(theta) * r[1];

			vertices.push_back(Vertex(glm::vec3(x[0], y[0], z[0]),
				glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec3(x[0], y[0], z[0])));
			vertices.push_back(Vertex(glm::vec3(x[1], y[1], z[1]),
				glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec3(x[1], y[1], z[1])));
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices.front(), GL_STATIC_DRAW);

	const char* vertexSource = GLSL(
		layout(location = 0) in vec3 position;
		layout(location = 1) in vec4 color;
		layout(location = 2) in vec3 normal;

		uniform mat4 trans;
		uniform mat4 view;
		uniform mat4 proj;

		out vec4 Color;

		void main() {
			Color = color;
			gl_Position = proj * (view * (trans * vec4(position, 1.0)));
		}
	);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, nullptr);
	glCompileShader(vertexShader);

	const char* fragmentSource = GLSL(
		in vec4 Color;

		out vec4 outColor;

		void main() {
			outColor = Color;
		}
	);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
	glCompileShader(fragmentShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	uniTrans = glGetUniformLocation(shaderProgram, "trans");
	glm::mat4 model = glm::mat4(1.0f);
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));

	glm::mat4 view = glm::lookAt(glm::vec3(0, 3.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	uniView = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(45.0f, 800.0f / 600.0f, 1.0f, 10.0f);
	uniProj = glGetUniformLocation(shaderProgram, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, (3 + 4 + 3)*sizeof(GLfloat), nullptr);

	colAttrib = glGetAttribLocation(shaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 4, GL_FLOAT, GL_FALSE, (3 + 4 + 3)*sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	normalAttrib = glGetAttribLocation(shaderProgram, "normal");
	glEnableVertexAttribArray(normalAttrib);
	glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, (3 + 4 + 3)*sizeof(GLfloat), (void*)((3 + 4) * sizeof(GLfloat)));

	glDisableVertexAttribArray(posAttrib);
	glDisableVertexAttribArray(colAttrib);
	glDisableVertexAttribArray(normalAttrib);

	glUseProgram(0);
}
Sphere::~Sphere()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(shaderProgram);
}

void Sphere::update()
{
}
void Sphere::draw()
{
	glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));

	glUseProgram(shaderProgram);

	glEnableVertexAttribArray(posAttrib);
	glEnableVertexAttribArray(colAttrib);
	glEnableVertexAttribArray(normalAttrib);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());

	glDisableVertexAttribArray(posAttrib);
	glDisableVertexAttribArray(colAttrib);
	glDisableVertexAttribArray(normalAttrib);
}