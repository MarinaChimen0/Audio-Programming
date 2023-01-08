#include "CatmullRom.h"
#define _USE_MATH_DEFINES
#include <math.h>

CCatmullRom::CCatmullRom()
{
	m_vertexCount = 0;
	w = 30.0f;
}

CCatmullRom::~CCatmullRom()
{}

// Perform Catmull Rom spline interpolation between four points, interpolating the space between p1 and p2
glm::vec3 CCatmullRom::Interpolate(glm::vec3 &p0, glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;

	glm::vec3 a = p1;
	glm::vec3 b = 0.5f * (-p0 + p2);
	glm::vec3 c = 0.5f * (2.0f*p0 - 5.0f*p1 + 4.0f*p2 - p3);
	glm::vec3 d = 0.5f * (-p0 + 3.0f*p1 - 3.0f*p2 + p3);

	return a + b*t + c*t2 + d*t3;

}


void CCatmullRom::SetControlPoints()
{
	// Set control points (m_controlPoints) here, or load from disk
	m_controlPoints.push_back(glm::vec3(100, 5, 0));
	m_controlPoints.push_back(glm::vec3(71, 5, 71));
	m_controlPoints.push_back(glm::vec3(0, 5, 100));
	m_controlPoints.push_back(glm::vec3(-71, 5, 71));
	m_controlPoints.push_back(glm::vec3(-100, 5, 0));
	m_controlPoints.push_back(glm::vec3(-71, 5, -71));
	m_controlPoints.push_back(glm::vec3(0, 5, -100));
	m_controlPoints.push_back(glm::vec3(71, 5, -71));

	// Optionally, set upvectors (m_controlUpVectors, one for each control point as well)
}


// Determine lengths along the control points, which is the set of control points forming the closed curve
void CCatmullRom::ComputeLengthsAlongControlPoints()
{
	int M = (int) m_controlPoints.size();

	float fAccumulatedLength = 0.0f;
	m_distances.push_back(fAccumulatedLength);
	for (int i = 1; i < M; i++) {
		fAccumulatedLength += glm::distance(m_controlPoints[i-1], m_controlPoints[i]);
		m_distances.push_back(fAccumulatedLength);
	}

	// Get the distance from the last point to the first
	fAccumulatedLength += glm::distance(m_controlPoints[M-1], m_controlPoints[0]);
	m_distances.push_back(fAccumulatedLength);
}


// Return the point (and upvector, if control upvectors provided) based on a distance d along the control polygon
bool CCatmullRom::Sample(float d, glm::vec3 &p, glm::vec3 up)
{
	if (d < 0)
		return false;

	int M = (int) m_controlPoints.size();
	if (M == 0)
		return false;


	float fTotalLength = m_distances[m_distances.size() - 1];

	// The the current length along the control polygon; handle the case where we've looped around the track
	float fLength = d - (int) (d / fTotalLength) * fTotalLength;

	// Find the current segment
	int j = -1;
	for (int i = 0; i < (int)m_distances.size(); i++) {
		if (fLength >= m_distances[i] && fLength < m_distances[i + 1]) {
			j = i; // found it!
			break;
		}
	}

	if (j == -1)
		return false;

	// Interpolate on current segment -- get t
	float fSegmentLength = m_distances[j + 1] - m_distances[j];
	float t = (fLength - m_distances[j]) / fSegmentLength;
	
	// Get the indices of the four points along the control polygon for the current segment
	int iPrev = ((j-1) + M) % M;
	int iCur = j;
	int iNext = (j + 1) % M;
	int iNextNext = (j + 2) % M;

	// Interpolate to get the point (and upvector)
	p = Interpolate(m_controlPoints[iPrev], m_controlPoints[iCur], m_controlPoints[iNext], m_controlPoints[iNextNext], t);
	if (m_controlUpVectors.size() == m_controlPoints.size())
		up = (glm::normalize(Interpolate(m_controlUpVectors[iPrev], m_controlUpVectors[iCur], m_controlUpVectors[iNext], m_controlUpVectors[iNextNext], t)));


	return true;
}



// Sample a set of control points using an open Catmull-Rom spline, to produce a set of iNumSamples that are (roughly) equally spaced
void CCatmullRom::UniformlySampleControlPoints(int numSamples)
{
	glm::vec3 p, up;

	// Compute the lengths of each segment along the control polygon, and the total length
	ComputeLengthsAlongControlPoints();
	float fTotalLength = m_distances[m_distances.size() - 1];

	// The spacing will be based on the control polygon
	float fSpacing = fTotalLength / numSamples;

	// Call PointAt to sample the spline, to generate the points
	for (int i = 0; i < numSamples; i++) {
		Sample(i * fSpacing, p, up);
		m_centrelinePoints.push_back(p);
		if (m_controlUpVectors.size() > 0)
			m_centrelineUpVectors.push_back(up);

	}


	// Repeat once more for truly equidistant points
	m_controlPoints = m_centrelinePoints;
	m_controlUpVectors = m_centrelineUpVectors;
	m_centrelinePoints.clear();
	m_centrelineUpVectors.clear();
	m_distances.clear();
	ComputeLengthsAlongControlPoints();
	fTotalLength = m_distances[m_distances.size() - 1];
	fSpacing = fTotalLength / numSamples;
	for (int i = 0; i < numSamples; i++) {
		Sample(i * fSpacing, p, up);
		m_centrelinePoints.push_back(p);
		if (m_controlUpVectors.size() > 0)
			m_centrelineUpVectors.push_back(up);
	}


}


void CCatmullRom::CreateCentreline()
{
	// Call Set Control Points
	SetControlPoints();

	// Call UniformlySampleControlPoints with the number of samples required
	UniformlySampleControlPoints(500);

	// Create a VAO called m_vaoCentreline and a VBO to get the points onto the graphics card
	glGenVertexArrays(1, &m_vaoCentreline);
	glBindVertexArray(m_vaoCentreline);

	// Create a VBO  
	CVertexBufferObject vbo;
	vbo.Create();
	vbo.Bind();
	glm::vec2 texCoord(0.0f, 0.0f);
	glm::vec3 normal(0.0f, 1.0f, 0.0f);
	for (unsigned int i = 0; i < m_centrelinePoints.size(); i++) {
		//_RPT1(0, "%d\n", m_centrelinePoints[i]);
		glm::vec3 v = m_centrelinePoints[i];
		vbo.AddData(&v, sizeof(glm::vec3));
		vbo.AddData(&texCoord, sizeof(glm::vec2));
		vbo.AddData(&normal, sizeof(glm::vec3));
	}

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


void CCatmullRom::CreateOffsetCurves()
{
	// Compute the offset curves, one left, and one right.  Store the points in m_leftOffsetPoints and m_rightOffsetPoints respectively
	for (unsigned int i = 0; i < m_centrelinePoints.size(); i++) {
		glm::vec3 p = m_centrelinePoints[i];
		glm::vec3 pNext = m_centrelinePoints[(i+1)%m_centrelinePoints.size()];
		//Normalised tangent vector T that points from p to pNext
		glm::vec3 t = glm::normalize(glm::vec3(pNext.x - p.x, pNext.y - p.y, pNext.z - p.z));
		//N = T x y. Replace t for interpolated up vector
		glm::vec3 n = glm::normalize(glm::cross(t, glm::vec3(0, 1, 0)));

		//store l
		m_leftOffsetPoints.push_back(p - (w / 2)*n);
		//store r
		m_rightOffsetPoints.push_back(p + (w / 2)*n);
	}

	// Generate two VAOs called m_vaoLeftOffsetCurve and m_vaoRightOffsetCurve, each with a VBO, and get the offset curve points on the graphics card
	// Note it is possible to only use one VAO / VBO with all the points instead.

	//Left side
	glGenVertexArrays(1, &m_vaoLeftOffsetCurve);
	glBindVertexArray(m_vaoLeftOffsetCurve);

	// Create a VBO  
	CVertexBufferObject vboLeft;
	vboLeft.Create();
	vboLeft.Bind();
	glm::vec2 texCoord(0.0f, 0.0f);
	glm::vec3 normal(0.0f, 1.0f, 0.0f);
	for (unsigned int i = 0; i < m_leftOffsetPoints.size(); i++) {
		glm::vec3 v = m_leftOffsetPoints[i];
		vboLeft.AddData(&v, sizeof(glm::vec3));
		vboLeft.AddData(&texCoord, sizeof(glm::vec2));
		vboLeft.AddData(&normal, sizeof(glm::vec3));
	}

	// Upload the VBO to the GPU  
	vboLeft.UploadDataToGPU(GL_STATIC_DRAW);

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
	
	//Right side
	glGenVertexArrays(1, &m_vaoRightOffsetCurve);
	glBindVertexArray(m_vaoRightOffsetCurve);

	// Create a VBO  
	CVertexBufferObject vboRight;
	vboRight.Create();
	vboRight.Bind();
	for (unsigned int i = 0; i < m_rightOffsetPoints.size(); i++) {
		glm::vec3 v = m_rightOffsetPoints[i];
		vboRight.AddData(&v, sizeof(glm::vec3));
		vboRight.AddData(&texCoord, sizeof(glm::vec2));
		vboRight.AddData(&normal, sizeof(glm::vec3));
	}

	// Upload the VBO to the GPU  
	vboRight.UploadDataToGPU(GL_STATIC_DRAW);

	// Set the vertex attribute locations  
	stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);

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


void CCatmullRom::CreateTrack()
{
	
	// Generate a VAO called m_vaoTrack and a VBO to get the offset curve points and indices on the graphics card
	glGenVertexArrays(1, &m_vaoTrack);
	glBindVertexArray(m_vaoTrack);

	// Create a VBO  
	CVertexBufferObject vbo;
	vbo.Create();
	vbo.Bind();
	unsigned int iL = 0;
	unsigned int iR = 0;
	for (unsigned int i = 0; i < m_leftOffsetPoints.size() + m_rightOffsetPoints.size(); i++) {
		if (i % 2 == 0) {
			glm::vec3 p = m_leftOffsetPoints[iL];
			glm::vec3 pNext = m_leftOffsetPoints[(iL + 1) % m_leftOffsetPoints.size()];
			//Normalised tangent vector T that points from p to pNext
			glm::vec3 t = glm::normalize(glm::vec3(pNext.x - p.x, pNext.y - p.y, pNext.z - p.z));
			//N = T x y. Replace y for interpolated up vector
			glm::vec3 n = glm::normalize(glm::cross(t, glm::vec3(0, 1, 0)));
			//B = N x T
			glm::vec3 b = glm::normalize(glm::cross(n, t));
			//text vector 
			glm::vec2 st = glm::vec2(0, iL*0.05);
			vbo.AddData(&p, sizeof(glm::vec3)); //pos
			vbo.AddData(&st, sizeof(glm::vec2)); //text
			vbo.AddData(&b, sizeof(glm::vec3)); //normal
			iL++;
		}
		else {
			glm::vec3 p = m_rightOffsetPoints[iR];
			glm::vec3 pNext = m_rightOffsetPoints[(iR + 1) % m_rightOffsetPoints.size()];
			//Normalised tangent vector T that points from p to pNext
			glm::vec3 t = glm::normalize(glm::vec3(pNext.x - p.x, pNext.y - p.y, pNext.z - p.z));
			//N = T x y. Replace y for interpolated up vector
			glm::vec3 n = glm::normalize(glm::cross(t, glm::vec3(0, 1, 0)));
			//B = N x T
			glm::vec3 b = glm::normalize(glm::cross(n, t));
			//text vector 
			glm::vec2 st = glm::vec2(1, iR*0.05);
			vbo.AddData(&p, sizeof(glm::vec3)); //pos
			vbo.AddData(&st, sizeof(glm::vec2)); //text
			vbo.AddData(&b, sizeof(glm::vec3)); //normal
			iR++;
		}
		m_vertexCount++;
	}

	//We want the path to loop, so that the end connects to the beginning.
	//For this, it will be necessary to generate two additional points in the VBO, which repeat the first two points.

	//Normalised tangent vector T that points from p to pNext
	glm::vec3 t = glm::normalize(glm::vec3(m_leftOffsetPoints[1].x - m_leftOffsetPoints[0].x, m_leftOffsetPoints[1].y - m_leftOffsetPoints[0].y, m_leftOffsetPoints[1].z - m_leftOffsetPoints[0].z));
	//N = T x y. Replace y for interpolated up vector
	glm::vec3 n = glm::normalize(glm::cross(t, glm::vec3(0, 1, 0)));
	//B = N x T
	glm::vec3 b = glm::normalize(glm::cross(n, t));
	//text vector 
	glm::vec2 st = glm::vec2(0, iL*0.05);
	vbo.AddData(&m_leftOffsetPoints[0], sizeof(glm::vec3)); //pos
	vbo.AddData(&st, sizeof(glm::vec2)); //text
	vbo.AddData(&b, sizeof(glm::vec3)); //normal

	//Normalised tangent vector T that points from p to pNext
	t = glm::normalize(glm::vec3(m_rightOffsetPoints[1].x - m_rightOffsetPoints[0].x, m_rightOffsetPoints[1].y - m_rightOffsetPoints[0].y, m_rightOffsetPoints[1].z - m_rightOffsetPoints[0].z));
	//N = T x y. Replace y for interpolated up vector
	n = glm::normalize(glm::cross(t, glm::vec3(0, 1, 0)));
	//B = N x T
	b = glm::normalize(glm::cross(n, t));
	//text vector 
	st = glm::vec2(1, iR*0.05);
	vbo.AddData(&m_rightOffsetPoints[0], sizeof(glm::vec3)); //pos
	vbo.AddData(&st, sizeof(glm::vec2)); //text
	vbo.AddData(&b, sizeof(glm::vec3)); //normal

	m_vertexCount = m_vertexCount + 2;

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


void CCatmullRom::RenderCentreline()
{
	// Bind the VAO m_vaoCentreline and render it
	glBindVertexArray(m_vaoCentreline);
	glLineWidth(2.0f);
	glDrawArrays(GL_POINTS, 0, m_centrelinePoints.size());
	glDrawArrays(GL_LINE_LOOP, 0, m_centrelinePoints.size());
}

void CCatmullRom::RenderOffsetCurves()
{
	// Bind the VAO m_vaoLeftOffsetCurve and render it
	glBindVertexArray(m_vaoLeftOffsetCurve);
	glLineWidth(2.0f);
	glDrawArrays(GL_POINTS, 0, m_leftOffsetPoints.size());
	glDrawArrays(GL_LINE_STRIP, 0, m_leftOffsetPoints.size());

	// Bind the VAO m_vaoRightOffsetCurve and render it
	glBindVertexArray(m_vaoRightOffsetCurve);
	glLineWidth(2.0f);
	glDrawArrays(GL_POINTS, 0, m_rightOffsetPoints.size());
	glDrawArrays(GL_LINE_STRIP, 0, m_rightOffsetPoints.size());
}


void CCatmullRom::RenderTrack()
{
	// Bind the VAO m_vaoTrack and render it
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(m_vaoTrack);
	m_texture.Bind();
	glLineWidth(2.0f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertexCount);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

int CCatmullRom::CurrentLap(float d)
{

	return (int)(d / m_distances.back());

}

void CCatmullRom::CreatePath(string a_sDirectory, string a_sFilename) {

	m_texture.Load(a_sDirectory + a_sFilename);

	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	GLfloat aniso;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	m_texture.SetSamplerObjectParameterf(GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
}