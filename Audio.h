#pragma once
#include <windows.h>									// Header File For The Windows Library
#include "./include/fmod_studio/fmod.hpp"
#include "./include/fmod_studio/fmod_errors.h"
#include "Common.h"
#include "CircBuffer.h"
#include "SoundSource.h"
#include "Camera.h"

class CCamera;

class CAudio
{

public:

	CAudio(); //Constructor 
	~CAudio(); //Destructor

	//Initialise function
	bool Initialise();

	//Functions for loading and playing the music stream - task 1
	bool LoadMusicStream(const char *filename);
	bool PlayMusicStream();

	//Functions for loading and playing the sound of the sound source - task 2 part 1
	bool LoadSoundSource(const char *filename);
	bool PlaySoundSource();

	//Functions for loading and playing the sound of the sound source - task 2 part 1
	bool LoadObjectSound(const char *filename);
	bool PlayObjectSound();

	//Update function
	void Update(float external_control, const CSoundSource *soundSource, const CSoundSource *submarineSoundSource, float submarine_vel, const CCamera *cam);

private:
		
	//FMOD_DSP_STATE struct 
	typedef struct
	{
		CBuffer *buffer;
		float external_control;
	} mydsp_data_t;

	//Custom DSP
	static FMOD_RESULT F_CALLBACK DSPCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels);
	//Callback to set float parameter of the DSP
	static FMOD_RESULT F_CALLBACK myDSPSetParameterFloatCallback(FMOD_DSP_STATE *dsp_state, int index, float value);
	//Function to create DSP 
	static FMOD_RESULT F_CALLBACK myDSPCreateCallback(FMOD_DSP_STATE *dsp_state);
	//Function to get the float parameter of the DSP - not used in the program, has been used to test the code
	static FMOD_RESULT F_CALLBACK myDSPGetParameterFloatCallback(FMOD_DSP_STATE *dsp_state, int index, float *value, char *valstr);

	//Takes a glm vector and returns it as a FMOD vector 
	FMOD_VECTOR *ToFmodVector(const glm::vec3 &v);
	// Function to check for error
	void FmodErrorCheck(FMOD_RESULT result);

	FMOD_RESULT result; //Variable to save the result of FmodErrorCheck
	FMOD::System *m_FmodSystem;	// the global variable for talking to FMOD
	
	//Music sound and channel
	FMOD::Sound *m_music;
	FMOD::Channel* m_musicChannel;

	//Module source sound and channel
	FMOD::Sound *m_sound1;
	FMOD::Channel *m_3dChannel1; // The channel containing the 3d sound effects

	//Submarine sound and channel
	FMOD::Sound *m_sound2;
	FMOD::Channel *m_3dChannel2; // The channel containing the 3d sound effects

	FMOD::DSP *m_dsp; //Music DSP
	FMOD::DSP *submarine_dsp; //Submarine DSP

	//Coefficients of the static FIR filters
	static vector<float> FIR1;
	static vector<float> FIR2;

};
