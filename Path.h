#pragma once
#include "Common.h"
#include "CatmullRom.h"

class Path {
private:
	vector<CCatmullRom> splines;
	GLuint m_vao;
	int control_points;
public:
	Path();
	~Path();
	//Path(vector<glm::vec3> &points);
	glm::vec3 Interpolate(glm::vec3 &p0, glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, float t);
	void CreatePath(vector<glm::vec3> points);
	void RenderPath();
};