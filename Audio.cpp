#include "Audio.h"

#pragma comment(lib, "lib/fmod_vc.lib")
#pragma warning(disable:4996)

//Static FIR filters 
vector<float> CAudio::FIR1{ 0.01473892f, 0.01595279f, 0.01473892f };
vector<float> CAudio::FIR2{ 0.2025804f,  0.52309322f, 0.2025804f };

CAudio::CAudio()
{}

CAudio::~CAudio()
{}

// Check for error
void CAudio::FmodErrorCheck(FMOD_RESULT result)
{
	if (result != FMOD_OK) {
		const char *errorString = FMOD_ErrorString(result);
		// MessageBox(NULL, errorString, "FMOD Error", MB_OK);
		// Warning: error message commented out -- if headphones not plugged into computer in lab, error occurs
	}
}

/*
	Custom DSP callback: 
	generate output samples based on the input and the dynamic filter coefficients
*/
FMOD_RESULT F_CALLBACK CAudio::DSPCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels)
{
	mydsp_data_t *thisdsp = (mydsp_data_t *)dsp_state->plugindata;

	for (int samp = 0; samp < length; samp++)
	{
		for (int chan = 0; chan < *outchannels; chan++)
		{		

			//Apply the final filter to the music, by convolution
			//Definition taken from the lecture 4 slides: f(x[n]) = ∑i=0 x[n−i]b[i]
			float conv = 0.0f;
			for (int i = 0; i < thisdsp->buffer->getLength(); ++i) {

				if ((samp - i)*inchannels + chan >= 0){

					float ec = thisdsp->external_control;
					//Interpolate the two filters based on the signal
					//Definition taken from lab 7: b_filt_mix = (1-mix_ratio) * b_filt1 + mix_ratio * b_filt2
					for (unsigned int i = 0; i < (thisdsp->buffer->getLength()); i++) {
						thisdsp->buffer->Put((1 - ec) * FIR1[i] + ec * FIR2[i]);
					}
					//The index is applied to the sample so the channels dont get mixed 
					conv = conv + inbuffer[(samp-i)*inchannels + chan] * thisdsp->buffer->ItemAt(i *inchannels + chan);
				}
				else {
					break;
				}
			}

			//The convolution result is saved in the outbuffer
			outbuffer[samp**outchannels + chan] = conv;

		}
	}

	return FMOD_OK;
}

/*
	Callback called when DSP is created. This implementation creates a structure which is attached to the dsp state's 'plugindata' member.
*/
FMOD_RESULT F_CALLBACK CAudio::myDSPCreateCallback(FMOD_DSP_STATE *dsp_state)
{

	//Allocates space for the dsp structure 
	mydsp_data_t *data = (mydsp_data_t *)calloc(sizeof(mydsp_data_t), 1);
	if (!data)
	{
		return FMOD_ERR_MEMORY;
	}

	//Plugindata pointer of the DSP state to the DSP structure
	dsp_state->plugindata = data;
	//Initialise the elements of the structure 
	data->external_control = 0.0f; 
	data->buffer = new CBuffer(CAudio::FIR1.size()); 

	return FMOD_OK;
}

/*
	Callback called when DSP::setParameterFloat is called. 
*/
FMOD_RESULT F_CALLBACK CAudio::myDSPSetParameterFloatCallback(FMOD_DSP_STATE *dsp_state, int index, float value)
{

	//If the index is the one asigned to the external control parameter 
	if (index == 1)
	{
		//Changes the value of the parameter
		mydsp_data_t *mydata = (mydsp_data_t *)dsp_state->plugindata;
		mydata->external_control = value;

		return FMOD_OK;
	}

	return FMOD_ERR_INVALID_PARAM;
}

/*
	 Callback called when DSP::getParameterFloat is called.
*/
FMOD_RESULT F_CALLBACK CAudio::myDSPGetParameterFloatCallback(FMOD_DSP_STATE *dsp_state, int index, float *value, char *valstr)
{
	//If the index is the one asigned to the external control parameter 
	if (index == 1)
	{

		//Makes the value pointer points to the external control parameter of the structure
		mydsp_data_t *mydata = (mydsp_data_t *)dsp_state->plugindata;
		*value = mydata->external_control;

		return FMOD_OK;
	}

	return FMOD_ERR_INVALID_PARAM;
}

//Initialise the FMOD system and creates the DSP effect
bool CAudio::Initialise()
{
	// Create an FMOD system
	result = FMOD::System_Create(&m_FmodSystem);
	FmodErrorCheck(result);
	if (result != FMOD_OK)
		return false;

	// Initialise the system
	result = m_FmodSystem->init(32, FMOD_INIT_NORMAL, 0);
	FmodErrorCheck(result);
	if (result != FMOD_OK)
		return false;

	// Set 3D settings
	result = m_FmodSystem->set3DSettings(1.0f, 1.0f, 1.0f); //doppler scale, distance factor, distance roll-off
	FmodErrorCheck(result);
	if (result != FMOD_OK)
		return false;

	// Create the DSP effect 
	{
		//Creates the DSP descriptor
		FMOD_DSP_DESCRIPTION dspdesc;
		memset(&dspdesc, 0, sizeof(dspdesc));

		//Creates the DSP parameters
		FMOD_DSP_PARAMETER_DESC data_desc;
		FMOD_DSP_PARAMETER_DESC  external_control_desc;
		FMOD_DSP_PARAMETER_DESC *paramdesc[2] =
		{
			&data_desc,
			&external_control_desc
		};
		FMOD_DSP_INIT_PARAMDESC_DATA(data_desc, "data", "", "data", FMOD_DSP_PARAMETER_DATA_TYPE_USER);
		FMOD_DSP_INIT_PARAMDESC_FLOAT(external_control_desc, "external control", "%", "external control in percent", 0, 1, 1);

		//Intialise some variables of the DSP descriptor 
		strncpy_s(dspdesc.name, "Control FIR filter", sizeof(dspdesc.name));
		dspdesc.version = 0x00010000;
		dspdesc.numinputbuffers = 2;
		dspdesc.numoutputbuffers = 2;
		dspdesc.read = DSPCallback;
		dspdesc.create = myDSPCreateCallback;
		dspdesc.setparameterfloat = myDSPSetParameterFloatCallback;
		dspdesc.numparameters = 2;
		dspdesc.paramdesc = paramdesc;

		//Creates the DSP for the music stream 
		result = m_FmodSystem->createDSP(&dspdesc, &m_dsp);
		FmodErrorCheck(result);

		if (result != FMOD_OK)
			return false;

		//Creates the DSP for the submarine sound
		result = m_FmodSystem->createDSP(&dspdesc, &submarine_dsp);
		FmodErrorCheck(result);

		if (result != FMOD_OK)
			return false;
	}

	return true;
}

// Load a music stream
bool CAudio::LoadMusicStream(const char *filename)
{
	result = m_FmodSystem->createStream(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_music);
	FmodErrorCheck(result);

	if (result != FMOD_OK) 
		return false;

	return true;
}

// Play a music stream
bool CAudio::PlayMusicStream()
{
	result = m_FmodSystem->playSound(m_music, NULL, false, &m_musicChannel);
	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	//Sets the 2D mode
	result = m_musicChannel->setMode(FMOD_2D);
	FmodErrorCheck(result);

	//Adds the DSP effect 
	result = m_musicChannel->addDSP(0, m_dsp);
	FmodErrorCheck(result);

	return true;
}


// Load a sound source
bool CAudio::LoadSoundSource(const char *filename)
{
	result = m_FmodSystem->createSound(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_sound1);
	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	return true;
}

// Play the sound source
bool CAudio::PlaySoundSource()
{
	result = m_FmodSystem->playSound(m_sound1, NULL, false, &m_3dChannel1);
	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	//Sets the 3D mode
	result = m_3dChannel1->setMode(FMOD_3D);
	FmodErrorCheck(result);

	return true;
}

// Load the object sound
bool CAudio::LoadObjectSound(const char *filename)
{
	result = m_FmodSystem->createSound(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_sound2);
	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	return true;

}

// Play the object sound
bool CAudio::PlayObjectSound()
{
	result = m_FmodSystem->playSound(m_sound2, NULL, false, &m_3dChannel2);
	m_3dChannel2->setVolume(10.0);
	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	//Sets the 3D mode
	result = m_3dChannel2->setMode(FMOD_3D);
	FmodErrorCheck(result);

	//Adds the DSP effect 
	result = m_3dChannel2->addDSP(0, submarine_dsp);
	FmodErrorCheck(result);

	return true;
}

void CAudio::Update(float external_control, const CSoundSource *soundSource, const CSoundSource *submarineSoundSource, float submarine_vel, const CCamera *cam)
{

	//Sets the external control of the music filtering (changed by F1/F2 buttons) 
	float ec = external_control;
	result = m_dsp->setParameterFloat(1, ec);
	FmodErrorCheck(result);

	//Sets the 3D attributes, given by the position and velocity of the sound source, to the module sound 
	FMOD_VECTOR *srcPos = ToFmodVector(soundSource->GetPosition());
	FMOD_VECTOR *srcVel = ToFmodVector(soundSource->GetVelocity());
	result = m_3dChannel1->set3DAttributes(srcPos, srcVel);
	FmodErrorCheck(result);

	//Sets the external control of submarine sound filtering (the speed of the submarine, changed by F3/F4) 
	float vel = submarine_vel*1000; //Changes the speed to 0-1 range
	result = submarine_dsp->setParameterFloat(1, vel);
	FmodErrorCheck(result);

	//Sets the 3D attributes, given by the position and velocity of the sound source, to the submarine sound
	FMOD_VECTOR *srcPosSub = ToFmodVector(submarineSoundSource->GetPosition());
	FMOD_VECTOR *srcVelSub = ToFmodVector(submarineSoundSource->GetVelocity());
	result = m_3dChannel2->set3DAttributes(srcPosSub, srcVelSub);
	FmodErrorCheck(result);

	//Set the attributes of the 3D listener with the camera data 
	FMOD_VECTOR *camPos = ToFmodVector(cam->GetPosition());
	FMOD_VECTOR *camStrafe = ToFmodVector(cam->GetStrafeVector());
	FMOD_VECTOR *camUp = ToFmodVector(cam->GetUpVector());
	result = m_FmodSystem->set3DListenerAttributes(0, camPos, 0, camStrafe, camUp);
	FmodErrorCheck(result);

	//Updates the system
	m_FmodSystem->update();

}

//Takes a glm vector and returns it as a FMOD vector 
FMOD_VECTOR *CAudio::ToFmodVector(const glm::vec3 &v)
{
	FMOD_VECTOR *fv = new FMOD_VECTOR();
	fv->x = v.x; fv->y = v.y; fv->z = v.z;
	return fv;
}