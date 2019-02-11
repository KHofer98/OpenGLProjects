#pragma once

/*
provided by DI Gerlinde Emsenhuber
MultiMediaTechnology
FH Salzburg
*/


#include <GLAD\glad.h>
#include <GLM\glm.hpp>

#include <memory>
#include <string>
#include <experimental\filesystem>

namespace fs = std::experimental::filesystem;
namespace cg2
{
	enum class TextureFilter : unsigned char { None, Bilinear, Trilinear };
	enum class TextureWrap : unsigned char { BorderColor, Clamp, Repeat };

	class Texture2D
	{

	public:

		Texture2D(Texture2D const& other) = delete;
		Texture2D& operator = (Texture2D const& other) = delete;

		static std::shared_ptr<Texture2D> import(fs::path const& path, const TextureFilter filter = TextureFilter::Bilinear, const TextureWrap wrap = TextureWrap::BorderColor, const glm::vec4 border = glm::vec4(0.0f));

		GLuint handle() const { return mHandle; }
		glm::uvec2 size() const { return glm::uvec2( mWidth, mHeight ); }

	private:

		Texture2D() = default;
		~Texture2D() = default;

		GLuint		mHandle = 0;
		unsigned	mWidth = 0;
		unsigned	mHeight = 0;

		struct Deleter
		{
			void operator()(Texture2D *& p) const;
		};

	};
}