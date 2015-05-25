#ifndef SPHERE_H_
#define SPHERE_H_

#ifndef M_PI
#define M_PI        3.14159265358979323846264338327950288
#endif // M_PI

class Vertex
{
public:
	Vertex(const glm::vec3& position, const glm::vec4& color, const glm::vec3& normal)
		: position(position), color(color), normal(normal)
	{

	}

	glm::vec3 position;
	glm::vec4 color;
	glm::vec3 normal;
};

class Sphere
{
public:
	Sphere();
	~Sphere();

	void update();
	void draw();

	void setPosition(const glm::vec3 &position) { this->position = position; }

private:
	std::vector<Vertex>		vertices;

	glm::vec3 position;

	GLuint vao, vbo;
	GLuint uniTrans, uniView, uniProj;
	GLuint shaderProgram;
	GLuint posAttrib, colAttrib, normalAttrib;
};

#endif // SPHERE_H_