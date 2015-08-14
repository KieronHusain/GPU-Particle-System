//Used from the framework Simon Scarle gave us for our 2nd year GEA projects

#include "input.h"

extern HWND g_hWnd;

Input::Input(HINSTANCE hInstance) :pDirectInput(NULL), pKeyboard(NULL), pMouse(NULL)
{
	//Direct Input Stuff
	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&pDirectInput, NULL);

	//keyboard
	hr = pDirectInput->CreateDevice(GUID_SysKeyboard, &pKeyboard, NULL);
	hr = pKeyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = pKeyboard->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	//mouse
	hr = pDirectInput->CreateDevice(GUID_SysMouse, &pMouse, NULL);
	hr = pMouse->SetDataFormat(&c_dfDIMouse);
	hr = pMouse->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	//zero all current states
	ZeroMemory(keyboardState, sizeof(keyboardState));
	ZeroMemory(prevkeyboardState, sizeof(prevkeyboardState));
	ZeroMemory(tempkeyboardState, sizeof(tempkeyboardState));
	ZeroMemory(&mouseState, sizeof(mouseState));
	ZeroMemory(&prevmouseState, sizeof(prevmouseState));
	ZeroMemory(&tempmouseState, sizeof(tempmouseState));
}

Input::~Input()
{
	//tidy away Direct Input Stuff
	if (pKeyboard)
	{
		pKeyboard->Unacquire();
		pKeyboard->Release();
	}
	if (pMouse)
	{
		pMouse->Unacquire();
		pMouse->Release();
	}
	if (pDirectInput) pDirectInput->Release();
}

void Input::Pole()
{
	ReadKeyboard();
	ReadMouse();
}

bool Input::ReadKeyboard()
{
	// Read the keyboard device.
	HRESULT hr = pKeyboard->GetDeviceState(sizeof(tempkeyboardState), (LPVOID)&tempkeyboardState);
	if (FAILED(hr))
	{
		// If the keyboard lost focus or was not acquired then try to get control back.
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			pKeyboard->Acquire();
		}
		else
		{
			return false;
		}
	}
	else
	{
		//if succesfully poled shift things around
		memcpy(prevkeyboardState, keyboardState, sizeof(keyboardState));
		memcpy(keyboardState, tempkeyboardState, sizeof(tempkeyboardState));
	}

	return true;

}

bool Input::ReadMouse()
{
	// Read the mouse device.
	HRESULT hr = pMouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&tempmouseState);
	if (FAILED(hr))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			pMouse->Acquire();
		}
		else
		{
			return false;
		}
	}
	else
	{
		//if succesfully poled shift things around
		memcpy(&prevmouseState, &mouseState, sizeof(DIMOUSESTATE));
		memcpy(&mouseState, &tempmouseState, sizeof(DIMOUSESTATE));
	}

	return true;

}