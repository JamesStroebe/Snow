#pragma once

#include "Snow/Render/Shader/Shader.h"

#include <glad/glad.h>

namespace Snow {
    namespace Render {
        class OpenGLShader : public Shader {
        public:
            OpenGLShader(ShaderType type, const std::string& path);

            uint32_t GetShaderID() const { return m_RendererID; }

            const ShaderType GetType() const override { return m_Type; }

        private:

            void CompileAsSPIRVBinary();

            GLenum GetShaderType(ShaderType type);

            void Load(const std::string& source);

            std::string ReadShaderFromFile(const std::string& path);

            ShaderType m_Type;
            std::string m_Name, m_Path;
            std::string m_Source;

            uint32_t m_RendererID;
        };
    }
}