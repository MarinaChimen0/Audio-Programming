#include "Path.h"
#include "Common.h"

//Constructor 
Path::Path() {}

//Destructor
Path::~Path() {}

glm::vec3 Path::Interpolate(glm::vec3 &p0, glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, float t) {
	glm::vec3 a = p1;
	glm::vec3 b = 0.5f*(p2 - p0);
	glm::vec3 c = 0.5f*(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3);
	glm::vec3 d = 0.5f*(-p0 + 3.0f*p1 - 3.0f*p2 + p3);

	return a + b * t + c * t*t + d * t*t*t;
}

void Path::CreatePath(vector<glm::vec3> points) {
	control_points = points.size();
	// Use VAO to store state associated with vertices  
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	// Create a VBO  
	CVertexBufferObject vbo;
	vbo.Create();
	vbo.Bind();
	glm::vec2 texCoord(0.0f, 0.0f);
	glm::vec3 normal(0.0f, 1.0f, 0.0f);

	for (size_t i = 0; i < control_points - 3; i++) {
		glm::vec3 p0 = points[i];
		glm::vec3 p1 = points[i + 1];
		glm::vec3 p2 = points[i + 2];
		glm::vec3 p3 = points[i + 3];
		for (unsigned int i = 0; i < 100; i++) {
			float t = (float)i / 100.0f;
			glm::vec3 v = Interpolate(p0, p1, p2, p3, t);
			vbo.AddData(&v, sizeof(glm::vec3));
			vbo.AddData(&texCoord, sizeof(glm::vec2));
			vbo.AddData(&normal, sizeof(glm::vec3));
		}
	}

	for (unsigned int i = 0; i < 100; i++) {
		float t = (float)i / 100.0f;
		glm::vec3 v = Interpolate(points[control_points - 3], points[control_points - 2], points[control_points - 1], points[0], t);
		vbo.AddData(&v, sizeof(glm::vec3));
		vbo.AddData(&texCoord, sizeof(glm::vec2));
		vbo.AddData(&normal, sizeof(glm::vec3));
	}

	for (unsigned int i = 0; i < 100; i++) {
		float t = (float)i / 100.0f;
		glm::vec3 v = Interpolate(points[control_points - 2], points[control_points - 1], points[0], points[1], t);
		vbo.AddData(&v, sizeof(glm::vec3));
		vbo.AddData(&texCoord, sizeof(glm::vec2));
		vbo.AddData(&normal, sizeof(glm::vec3));
	}

	for (unsigned int i = 0; i < 100; i++) {
		float t = (float)i / 100.0f;
		glm::vec3 v = Interpolate(points[control_points - 1], points[0], points[1], points[2], t);
		vbo.AddData(&v, sizeof(glm::vec3));
		vbo.AddData(&texCoord, sizeof(glm::vec2));
		vbo.AddData(&normal, sizeof(glm::vec3));
	}

	vbo.AddData(&points[1], sizeof(glm::vec3));
	vbo.AddData(&texCoord, sizeof(glm::vec2));
	vbo.AddData(&normal, sizeof(glm::vec3));

	// Upload the VBO to the GPU  
	vbo.UploadDataToGPU(GL_STATIC_DRAW);

	// Set the vertex attribute locations  
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);

	// Vertex positions  
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates  
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors  
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void Path::RenderPath() {
	//Bind the VAO
	glBindVertexArray(m_vao);
	glLineWidth(10.0f);
	glDrawArrays(GL_LINE_STRIP, 0, 100*control_points + 1);
}


/**
Path::Path(vector<glm::vec3> &points) {
	CCatmullRom cr = CCatmullRom();
	for (size_t i = 0; i < points.size() - 3; i++) {
		cr.CreatePath(points[i], points[i + 1], points[i + 2], points[i + 3]);
		splines.push_back(cr);
	}
	cr.CreatePath(points[points.size() - 3], points[points.size() - 2], points[points.size() - 1], points[0]);
	splines.push_back(cr);
	cr.CreatePath(points[points.size() - 2], points[points.size() - 1], points[0], points[1]);
	splines.push_back(cr);
	cr.CreatePath(points[points.size()-1], points[0], points[1], points[2]);
	splines.push_back(cr);
}

void Path::RenderPath() {
	for (size_t i = 0; i < splines.size(); i++) {
		splines[i].RenderPath();
	}
}
*/