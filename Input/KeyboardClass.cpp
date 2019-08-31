#include "KeyboardClass.h"



KeyboardClass::KeyboardClass()
{
	for (int i = 0; i < 256; i++)
	{
		this->KeyStates[i] = false;
	}
}


KeyboardClass::~KeyboardClass()
{
}

bool KeyboardClass::KeyIsPressed(const unsigned char keycode)
{
	return this->KeyStates[keycode];
}

bool KeyboardClass::KeyBufferIsEmpty()
{
	return this->KeyBuffer.empty();
}

bool KeyboardClass::CharBufferIsEmpty()
{
	return this->charBuffer.empty();
}

KeyboardEvent KeyboardClass::ReadKey()
{
	if (this->KeyBuffer.empty())
	{
		return KeyboardEvent();
	}

	else
	{
		KeyboardEvent keyEvent = this->KeyBuffer.front();
		this->KeyBuffer.pop();
		return keyEvent;
	}
}

unsigned char KeyboardClass::ReadChar()
{
	if (this->charBuffer.empty())
	{
		return  0u;
	}

	else
	{
		unsigned char CharEvent = this->charBuffer.front();
		this->charBuffer.pop();
		return CharEvent;
	}
}

void KeyboardClass::OnKeyPressed(const unsigned char key)
{
	this->KeyStates[key] = true;
	this->KeyBuffer.push(KeyboardEvent(KeyboardEvent::EventType::Press, key));
}

void KeyboardClass::OnKeyReleased(const unsigned char key)
{
	this->KeyStates[key] = true;
	this->KeyBuffer.push(KeyboardEvent(KeyboardEvent::EventType::Release, key));
}

void KeyboardClass::OnChar(const unsigned char key)
{
	this->charBuffer.push(key);
}

void KeyboardClass::EnableAutoRepeatKeys()
{
	this->autoRepeatKeys = true;
}

void KeyboardClass::DisableAutoRepeatKeys()
{
	this->autoRepeatKeys = false;
}

void KeyboardClass::EnableAutoRepeatChars()
{
	this->autuoReapeatChars = true;
}

void KeyboardClass::DisableAutoRepeatChars()
{
	this->autuoReapeatChars = false;
}

bool KeyboardClass::IsKeysAutoRepeat()
{
	return this->autoRepeatKeys;
}

bool KeyboardClass::IsCharsAutoRepeat()
{
	return this->autuoReapeatChars;
}
