#pragma once
#include "Common.h"

// Holds information useful for a 3d sound effect
class CSoundSource
{
public:

	CSoundSource(); //Constructor
	~CSoundSource(); //Destructor

	void SetPosition(glm::vec3 pos);	// Sets the position
	glm::vec3 GetPosition() const;	// Returns the position
	void SetVelocity(glm::vec3 vel);	// Sets the velocity
	glm::vec3 GetVelocity() const;	// Returns the velocity 

	// Updates the position based on velocity and the amount of time between frames
	void Update(float dt);

private:

	glm::vec3 m_pos;	// Location vector in world coordinates
	glm::vec3 m_vel;	// Velocity vector

};