#include <spch.h>

#include "Snow/Render/Material.h"

namespace Snow {
	namespace Render {

		Ref<Material> Material::Create(const Ref<Shader>& shader) {
			return Ref<Material>::Create(shader);
		}

		Material::Material(const Ref<Shader>& shader) :
			m_Shader(shader) {
			AllocateStorage();
		}

		void Material::AllocateStorage() {
			const auto& uniformBuffer = m_Shader->GetUniformBuffer("Material");

			m_UniformStorageBuffer.Allocate(uniformBuffer.Size);
			m_UniformStorageBuffer.ZeroInitialize();
		}

		const ShaderUniform* Material::FindUniformDecl(const std::string& name) {
			const auto& uniformBuffer = m_Shader->GetUniformBuffer("Material");

			for (uint32_t i = 0; i < uniformBuffer.Uniforms.size(); i++) {
				if (uniformBuffer.Uniforms[i].GetName() == name) {
					return &uniformBuffer.Uniforms[i];
				}
			}
			return nullptr;
		}

		const ShaderResource* Material::FindResourceDecl(const std::string& name) {
			auto& resources = m_Shader->GetResources();
			for (const auto& [n, resource] : resources) {
				if (resource.GetName() == name)
					return &resource;
			}

			return nullptr;
		}

		void Material::Bind() {
			m_Shader->Bind();

			m_Shader->SetUniformBufferData("Material", m_UniformStorageBuffer.Data, m_UniformStorageBuffer.Size);

			BindTextures();
		}

		void Material::BindTextures() {
			for (size_t i = 0; i < m_Textures.size(); i++) {
				auto& texture = m_Textures[i];
				if (texture)
					texture->Bind(i);
			}
		}

		Ref<MaterialInstance> MaterialInstance::Create(const Ref<Material>& material) {
			return Ref<MaterialInstance>::Create(material);
		}

		MaterialInstance::MaterialInstance(const Ref<Material>& material, const std::string& name) :
			m_Material(material), m_Name(name) {
			m_Material->m_MaterialInstances.insert(this);
			AllocateStorage();
		}

		MaterialInstance::~MaterialInstance() {
			m_Material->m_MaterialInstances.erase(this);
		}

		void MaterialInstance::AllocateStorage() {
			const auto& uniformBuffer = m_Material->GetShader()->GetUniformBuffer("Material");

			m_UniformStorageBuffer.Allocate(uniformBuffer.Size);
			m_UniformStorageBuffer.ZeroInitialize();
		}

		void MaterialInstance::OnMaterialValueUpdated(const ShaderUniform& uniform) {
			if (m_OverriddenValues.find(uniform.GetName()) == m_OverriddenValues.end()) {
				auto& buffer = m_UniformStorageBuffer;
				auto& materialBuffer = m_Material->m_UniformStorageBuffer;
				buffer.Write(materialBuffer.Data + uniform.GetOffset(), uniform.GetSize(), uniform.GetOffset());
			}
		}

		void MaterialInstance::Bind() {
			m_Material->GetShader()->Bind();

			m_Material->GetShader()->SetUniformBufferData("Material", m_UniformStorageBuffer.Data, m_UniformStorageBuffer.Size);
			m_Material->BindTextures();

			for (size_t i = 0; i < m_Textures.size(); i++) {
				auto& texture = m_Textures[i];
				if (texture)
					texture->Bind(i);
			}
		}
	}
}