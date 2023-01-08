#pragma once
#include "Common.h"

// A circular buffer
class CBuffer
{
public:
	CBuffer(int l); //Initialises the class attributes with the lengt passes as a parameter
	~CBuffer(); //Destructor
	void Put(float n); //Adds an item 
	float ItemAt(int i); //Returns the item at the position i
	//Getters of the class attributes
	int getTail();
	int getLength();

private:
	float* buffer;  // The data 
	int length;	// The length of the buffer
	int tail;	// The next element's index
};