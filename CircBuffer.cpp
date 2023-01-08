#include "CircBuffer.h"

//Initialises the class attributes with the lengt passes as a parameter
CBuffer::CBuffer(int l)
{
	length = l;
	tail = 0;
	buffer = new float[length];
}

CBuffer::~CBuffer()
{
	//Deletes the buffer
	delete buffer;
}

//Adds an item 
void CBuffer::Put(float n)
{
	buffer[tail % length] = n;
	tail++; //The tail increases
}

//Returns the item at the position i
float CBuffer::ItemAt(int i)
{
	if (i >= 0)
		return buffer[i % length];
	else
		return NULL;
}

//Getters of the class attributes 
int CBuffer::getTail() { return tail; }
int CBuffer::getLength() { return length; }