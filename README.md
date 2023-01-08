# Audio-Programming
This project was implemented for the Digital Signal Processing and Audio Programming module in the MSc in Computer Games Technology that I completed in 2020. Programming in C++ with Visual Studio, we had two tasks:

### Implement a dynamic filter using FMOD.

The dynamic FIR filter has been created by interpolating two static FIR filters. Since there is no filter design function available in FMOD, two arrays of FIR filter coefficients have been created by using in Python the “firls” function of the SciPy API. After testing different filter lengths and parameters of this function, the most appropriate filters have been the ones created by the following code:

´´´Python
filt_len = 3
samplerate = 44100
b_filt2 = signal.firls(filt_len, [0, 100, 700, 1000, 1600, sa
mplerate/2], [1, 0, 1, 0, 1, 0], fs=samplerate)
b_filt1 = signal.firls(filt_len, [0, 2000, 2600, 2900, 3500,
samplerate/2], [0, 0, 1, 1, 0, 0], fs=samplerate)
```

In second place, for saving the coefficients of the dynamic filter, a circular buffer has been implemented. The code is in the files CircBuffer.h and CircBuffer.cpp of the project. The dynamic FIR filtering has been implemented in the CAudio class (Audio.h/Audio.cpp) of the OpenGlTemplate project using the FMOD API DSP system. The coefficients of the static filters have been hard coded and saved in two float vectors, FIR1 and FIR2. In order to make the dynamic filter response to the external control, a FMOD_DSP_STATE structure has been defined. It is called mydsp_data_t and it is defined in the header file. To instantiate and modify it,the following callback functions have been implemented:

```C++
static FMOD_RESULT F_CALLBACK myDSPCreateCallback(FMOD_DSP_STATE *dsp_state);
static FMOD_RESULT F_CALLBACK myDSPSetParameterFloatCallback(FMOD_DSP_STATE *dsp_state, int index, float value);
```

The coefficients of the dynamic filter have been calculated by interpolating the two static filters, and this final filter has been applied to the music stream by convolution. Both calculations have been implemented in a DSP callback:

```C++
static FMOD_RESULT F_CALLBACK DSPCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels);
```

In the Initialise function of the CAudio class, the FMOD System and the DSP effect are created. The DSP effect is applied to the music FMOD channel in thePlayMusicStream function. The Update function receives a float, the external control, that is set as the float parameter of the DSP. This external control is declared and initialised in the Game class (Game.cpp/Game.h) and gets modifiedin its ProcessEvents function, when the F1/F2 keys are pressed. An object of the CAudio class is also declared and initialised in the Game class and is updated in the Update method.

### Place a sound source in a 3D world where it can be moved around using the mouse and/or keyboard.
