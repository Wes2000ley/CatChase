#include "texture.h"
#include <iostream>

Texture2D::Texture2D()
{
	glGenTextures(1, &this->ID);
}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &this->ID);
}

void Texture2D::SetFormat(GLenum internalFormat, GLenum imageFormat)
{
	this->Internal_Format = internalFormat;
	this->Image_Format = imageFormat;
}

void Texture2D::Generate(unsigned int width, unsigned int height, const unsigned char* data)
{
	this->Width = width;
	this->Height = height;

	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, width, height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, this->ID);
}
