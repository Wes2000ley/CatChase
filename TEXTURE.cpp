/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include <iostream>
#include <GL/gl.h>

#include "texture.h"


Texture2D::Texture2D()
	: Width(0), Height(0),
	  Internal_Format(GL_RGB), Image_Format(GL_RGB),
	  Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT),
	  Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR)
{
	glGenTextures(1, &ID);
}

void Texture2D::Generate(unsigned int width, unsigned int height,
						 const unsigned char* data)
{
	Width  = width;
	Height = height;

	glBindTexture(GL_TEXTURE_2D, ID);

	// ALWAYS use tight packing so any image width is safe.
	GLint prevAlign;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevAlign);  // save
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);           // ← magic fix

	glTexImage2D(GL_TEXTURE_2D, 0,
				 Internal_Format,
				 width, height, 0,
				 Image_Format, GL_UNSIGNED_BYTE, data);

	glPixelStorei(GL_UNPACK_ALIGNMENT, prevAlign);   // restore

	// Pixel-perfect sampling (sharp UI & sprites)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, this->ID);
}
