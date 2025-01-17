#pragma once

#include "Graphics/API/Device.hpp"
#include "Graphics/RenderPass/DeferredGeometryPass.hpp"
#include "Graphics/RenderPass/ShadowPass.hpp"
#include "Graphics/RenderPass/BloomPass.hpp"

#include "Scene/Scene.hpp"

#include "Core/Log.hpp"

namespace helios::editor
{
	// Used for providing interface to log to ImGui console window.
	// Code is literally a heavily stripped down version from the ImGui_Demo.cpp file, for a basic logger.
	// reference : https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp
	
	// The editor handles all UI related task.
	// The goal is to avoid having UI specific configurations / settings in other abstractions, and have everything contained within this class.
	class Editor
	{
	public:
		Editor(const gfx::Device* device);
		~Editor();

		// Goal is to call this single function from the engine which does all the UI internally, helps make the engine clean as well.
		// This function is heavy WIP and not given as much importance as other abstractions.
		void Render(gfx::Device* const device, scene::Scene* const scene, gfx::DeferredPassRTs* const deferredPassRTs, gfx::ShadowPass* shadowPass, std::span<float, 4> clearColor, PostProcessBuffer& postProcessBufferData, const gfx::RenderTarget* renderTarget, gfx::GraphicsContext* graphicsContext);

		void OnResize(Uint2 dimensions) const;

		void ShowUI(bool value);

	private:
		void RenderSceneHierarchy(std::span<std::unique_ptr<helios::scene::Model>> models) const;
		
		// Handles camera and other scene related properties.
		void RenderSceneProperties(scene::Scene* scene) const;
		void RendererProperties(std::span<float, 4> clearColor, PostProcessBuffer& postProcessBufferData) const;

		void RenderLightProperties(std::vector<std::unique_ptr<helios::scene::Light>>& lights) const;

		void RenderDeferredGPass(gfx::Device* device, const gfx::DeferredPassRTs* deferredRTs) const;
		void RenderShadowPass(gfx::Device* device, gfx::ShadowPass* shadowPass);
		// void RenderBloomPass(gfx::Device* device, gfx::BloomPass* bloomPass);

		void RenderLogWindow();

		// Accepts the pay load (accepts data which is dragged in from content browser to the scene view port, and loads the model (if path belongs to a .gltf file).
		void RenderSceneViewport(const gfx::Device* device, const gfx::RenderTarget* renderTarget, scene::Scene* scene) const;

		// Handle drag and drop of models into viewport at run time.
		void RenderContentBrowser();
	
	private:
		bool mShowUI{ true };

		// Member variables for content browser.
        static inline std::filesystem::path mAssetsPath{};
		std::filesystem::path mContentBrowserCurrentPath{};

		// Path to .ini file.
		std::string mIniFilePath{};

	};
}