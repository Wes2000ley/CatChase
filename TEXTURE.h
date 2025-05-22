#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

// Texture2D manages a 2D OpenGL texture.
class Texture2D
{
public:
	Texture2D();
	~Texture2D();

	void Generate(unsigned int width, unsigned int height, const unsigned char* data);
	void Bind() const;

	void SetFormat(GLenum internalFormat, GLenum imageFormat);

	[[nodiscard]] unsigned int GetID() const { return ID; }
	[[nodiscard]] unsigned int GetWidth() const { return Width; }
	[[nodiscard]] unsigned int GetHeight() const { return Height; }

private:
	unsigned int ID = 0;
	unsigned int Width = 0, Height = 0;
	GLenum Internal_Format = GL_RGB;
	GLenum Image_Format = GL_RGB;
	GLenum Wrap_S = GL_REPEAT;
	GLenum Wrap_T = GL_REPEAT;
	GLenum Filter_Min = GL_LINEAR;
	GLenum Filter_Max = GL_LINEAR;
};

#endif // TEXTURE_H
