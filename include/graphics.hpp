#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <memory>
#include <utility>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "mesh.hpp"
#include "framebuffer.hpp"
#include "shader_light.hpp"

enum class SHADOW_QUALITY
{
	OFF = 0,
	MED = 512,
	HIGH = 1024,
	ULTRA = 2048
};

class Graphics
{
	public:

		Graphics(int width, int height);
		void setShadowQuality(SHADOW_QUALITY quality);
		SHADOW_QUALITY getShadowQuality();
		void setLightView(glm::mat4 view);
		glm::mat4 & getLightView();
		glm::mat4 & getOrthoProjection();
		glm::mat4 & getOmniPerspProjection();
		glm::mat4 getSpotPerspProjection(float outerCutOff);
		Shader & getBlinnPhongShader();
		Shader & getPostProcessingShader();
		Shader & getShadowMappingShader();
		std::unique_ptr<Framebuffer> & getMultisampleFBO();
		std::unique_ptr<Framebuffer> & getNormalFBO();
		std::unique_ptr<Framebuffer> & getOmniDepthFBO(int index);
		std::unique_ptr<Framebuffer> & getStdDepthFBO(int index);
		std::unique_ptr<Mesh> & getQuadMesh();
		void resizeScreen(int width, int height);

	private:

		std::unique_ptr<Framebuffer> multisample; // color + depth + stencil
		std::unique_ptr<Framebuffer> normal; // only color, no multisampling
		std::array<std::unique_ptr<Framebuffer>, 10> omniDepth; // for omnidirectional shadow mapping
		std::array<std::unique_ptr<Framebuffer>, 10> stdDepth; // for directional and spotlight shadow mapping

		SHADOW_QUALITY shadowQuality;
		glm::mat4 orthoProjection; // for directional light
		glm::mat4 omniPerspProjection; // for point lights

		Shader blinnPhong;
		Shader postProcessing;
		Shader shadowMapping;

		std::unique_ptr<Mesh> quad;
		Material quadMaterial;
};

#endif
