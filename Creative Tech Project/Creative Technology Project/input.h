//Used from the framework Simon Scarle gave us for our 2nd year GEA projects

#ifndef _INPUT_H_
#define _INPUT_H_
#include <windows.h>
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <dinput.h>

class Input
{
public:
	Input(HINSTANCE hInstance);
	~Input();

	void Pole();

	unsigned char kState(int test){ return keyboardState[test]; }
	unsigned char prevkState(int test){ return prevkeyboardState[test]; }
	DIMOUSESTATE* mState(){ return &mouseState; }
	DIMOUSESTATE* prevmState(){ return &prevmouseState; }

private:
	IDirectInput8*			pDirectInput;
	IDirectInputDevice8*	pKeyboard;
	IDirectInputDevice8*	pMouse;

	//current state
	unsigned char			keyboardState[256];
	DIMOUSESTATE			mouseState;

	//temp state
	unsigned char			tempkeyboardState[256];
	DIMOUSESTATE			tempmouseState;

	//previous state
	unsigned char			prevkeyboardState[256];
	DIMOUSESTATE			prevmouseState;

	bool ReadKeyboard();
	bool ReadMouse();
};

#endif