#pragma once

#include "Snow/Render/API/Texture.h"

#include "Snow/Platform/DirectX11/DirectXCommon.h"

namespace Snow {
	class DirectX11Texture2D : public Render::API::Texture2D {
	public:
		DirectX11Texture2D(Render::API::TextureFormat format, uint32_t width, uint32_t height, Render::API::TextureWrap wrap);
		DirectX11Texture2D(const std::string& path, bool srgb);

		void Bind(uint32_t slot) const override;
		Render::API::TextureFormat GetFormat() const override { return m_Format; }

		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }

		void ResizeBuffer(uint32_t width, uint32_t height) override;

		void Lock() override;
		void Unlock() override;

		void SetData(void* data, uint32_t size) override;

		uint32_t GetRendererID() const { return 0; }

		Buffer GetWriteableBuffer() override { return {}; }

		const std::string& GetPath() const override { return m_Path; }
	private:

		uint32_t m_Width, m_Height;
		std::string m_Path;

		Buffer m_ImageData;

		bool m_Locked = false;

		D3D11_TEXTURE2D_DESC m_TextureDesc;
		ID3D11Texture2D* m_Texture;
		ID3D11ShaderResourceView* m_ResourceView;
		
		D3D11_SAMPLER_DESC m_SamplerDesc;
		ID3D11SamplerState* m_SamplerState;

		Render::API::TextureFormat m_Format;
	};
}