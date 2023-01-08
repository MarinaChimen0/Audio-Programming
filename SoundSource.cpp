#include "SoundSource.h"

CSoundSource::CSoundSource()
{
	//Initilises the attributes 
	m_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	m_vel = glm::vec3(0.0f);
}

CSoundSource::~CSoundSource(){}

void CSoundSource::Update(float dt)
{
	//Changes the position based on the velocity and the amount of time between frames
	m_pos += m_vel * dt;
}

//Getters and setters of the class attributes 
void CSoundSource::SetPosition(glm::vec3 pos){ m_pos = pos; }
glm::vec3 CSoundSource::GetPosition() const { return m_pos; }
void CSoundSource::SetVelocity(glm::vec3 vel){ m_vel = vel; }
glm::vec3 CSoundSource::GetVelocity() const{ return m_vel; }
