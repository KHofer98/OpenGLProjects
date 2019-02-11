/*
provided by DI Gerlinde Emsenhuber
MultiMediaTechnology
FH Salzburg
*/

#include "texture2D.h"

#include "stb_image.h"

#include <iostream>

namespace cg2
{
	std::shared_ptr<Texture2D> Texture2D::import(fs::path const& path, const TextureFilter filter, const TextureWrap wrap, const glm::vec4 border)
	{
		std::shared_ptr<Texture2D> result(new Texture2D, Texture2D::Deleter());

		try
		{
			stbi_set_flip_vertically_on_load(true);
			int width, height, nrChannels;
			unsigned char *data = stbi_load(path.string().c_str(), &width, &height, &nrChannels, 0);
			if (!data)
			{
				std::cout << "import error: could not open image " << path << std::endl;
			}
			else
			{
				glGenTextures(1, &(result->mHandle));
				glBindTexture(GL_TEXTURE_2D, result->mHandle);

				switch (nrChannels)
				{
				case 4:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
					break;
				case 3:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
					break;
				case 2:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, data);
					break;
				default:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
					break;
				}
		
				if (filter == TextureFilter::Bilinear)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				}
				else if (filter == TextureFilter::Trilinear)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glGenerateMipmap(GL_TEXTURE_2D);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				}

				if (wrap == TextureWrap::Repeat)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				}
				else if (wrap == TextureWrap::Clamp)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
					glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &border[0]);
				}

				glBindTexture(GL_TEXTURE_2D, 0);

				result->mWidth = width;
				result->mHeight = height;
			}
			stbi_image_free(data);
		}
		catch (std::exception &e)
		{
			std::cout << "import error " << e.what() << std::endl;
		}

		return result;
	}

	void Texture2D::Deleter::operator()(Texture2D *& p) const
	{
		if (p == nullptr)
			return;

		glDeleteTextures(1, std::addressof(p->mHandle));

		delete p;
		p = nullptr;
	}
}
