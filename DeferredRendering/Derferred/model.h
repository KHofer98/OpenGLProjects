#pragma once

/*
provided by DI Gerlinde Emsenhuber
MultiMediaTechnology
FH Salzburg
*/

#include <GLAD\glad.h>
#include <GLM\glm.hpp>
#include <GLM\common.hpp>

#include <map>
#include <vector>
#include <memory>
#include <experimental\filesystem>

namespace fs = std::experimental::filesystem;

namespace cg2
{
	class GlslProgram;
	class Texture2D;

	enum class Attrib : unsigned char { POSITION, NORMAL, TANGENT, BITANGENT, TEXCOORD };
	typedef unsigned int AttribLocation;
	typedef std::map< Attrib, AttribLocation > AttribBindings;

	struct VertexData
	{
		glm::vec3					mPosition;
		glm::vec3					mNormal;
		glm::vec3					mTangent;
		glm::vec3					mBitangent;
		glm::vec2					mTexcoord;
	};

	struct Material
	{
		glm::vec3 mKDiffuse;
		glm::vec3 mKSpecular;
		glm::vec3 mKAmbient;
		float mShininess;
		float mOpacity;
		float mRefractivity;
		fs::path mDiffuseTexture;
		fs::path mSpecularTexture;
		fs::path mAOTexture;
		fs::path mNormalMap;
	};

	struct MeshData
	{
		unsigned					mVertexCount = 0;
		unsigned					mIndexCount = 0;
		Material					mMaterial;
	};

	struct MeshDataGPU
	{
		GLuint						mHandleToVertexBuffer = 0;
		GLuint						mHandleToIndexBuffer = 0;
	};

	// vertex array objects are separated form the rest ( -> buildVertexArrayObjects )
	// this was done because at the time, I needed to functionality to render the same asset
	// with a varienty of different shaders, with different attribute bindings

	class Model
	{

	public:

		Model(Model const& other) = delete;
		Model& operator = (Model const& other) = delete;

		// imports asset, uploads meshes & textures to gpu and builds default vertex array objects
		// meshes must provide position data (duh) as well as normals. uvs and tangent space vectors
		// are optional and will be initialized to default values if missing
		static std::shared_ptr< const Model > import(const fs::path path);

		struct Node
		{
			std::string					mName;
			glm::mat4					mOffsetTf;				// model tf relative to parent node
			glm::mat4					mModelTf;				// absolute model tf
			std::vector< unsigned >		mMeshIds;				// all meshes referenced by this node --> can be used to index mVAOs, mMeshDataCPU, mMeshDataGPU, mMaterials
			Node						*mParent = nullptr;
			std::vector< Node * >		mChildren;
		};

		std::vector< Node * >									mScenegraph;	// level-order traversal
		std::map< fs::path, std::shared_ptr< Texture2D > >		mTextures;
		std::vector< unsigned >									mVAOs;			// vertex array objects, using default attrib bindings
		std::vector< MeshDataGPU >								mMeshDataGPU;	// mesh data @ gpu
		std::vector< MeshData >									mMeshDataCPU;	// mesh data @ cpu
		std::vector< Material >									mMaterials;

		std::vector< GLuint > buildVertexArrayObjects(const AttribBindings bindings = { { Attrib::POSITION, 0 },
																						{ Attrib::NORMAL, 1 },
																						{ Attrib::TANGENT, 2 },
																						{ Attrib::BITANGENT, 3 },
																						{ Attrib::TEXCOORD, 4 } }) const;
	private:

		Model() = default;
		~Model() = default;

		struct Deleter
		{
			void operator()(Model *& p) const;
		};

	};

	class ModelRenderPass
	{
	public:
		virtual void operator()(std::shared_ptr<const Model>, std::shared_ptr<GlslProgram>, glm::mat4 const& tf) = 0;
		virtual ~ModelRenderPass() {}
	};
}