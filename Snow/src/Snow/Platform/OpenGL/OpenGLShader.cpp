#include <spch.h>

#include "Snow/Platform/OpenGL/OpenGLShader.h"

#include <glad/glad.h>

#include <shaderc/shaderc.hpp>

namespace Snow {
    namespace Render {
        shaderc_shader_kind ShadercKindFromShaderType(ShaderType type) {
            switch(type){
                case ShaderType::Vertex:    return shaderc_shader_kind::shaderc_glsl_vertex_shader;
                case ShaderType::Pixel:     return shaderc_shader_kind::shaderc_glsl_fragment_shader;
            }
        }

        OpenGLShader::OpenGLShader(ShaderType type, const std::string& path) :
            m_Path(path), m_Type(type) {

            m_Source = ReadShaderFromFile(m_Path);
            CompileAsSPIRVBinary();
            //Load(m_Source);

        }

        void OpenGLShader::Load(const std::string& source) {
            m_RendererID = glCreateShader(GetShaderType(m_Type));
            const GLchar* sourceC = (const GLchar*)m_Source.c_str();
            glShaderSource(m_RendererID, 1, &sourceC, 0);

            glCompileShader(m_RendererID);

            GLint isCompiled = 0;
            glGetShaderiv(m_RendererID, GL_COMPILE_STATUS, &isCompiled);
            if(isCompiled == GL_FALSE) {
                GLint maxLength = 0;
                glGetShaderiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);
                SNOW_CORE_ERROR("Shader compilation failed {0}, Shader Path {1}", &infoLog[0], m_Path);
                glDeleteShader(m_RendererID);
            }
        }

        std::vector<uint32_t> OpenGLShader::CompileAsSPIRVBinary() {
            shaderc::Compiler compiler;
            shaderc::CompileOptions options;

            const bool optimize = true;
            if(optimize)
                options.SetOptimizationLevel(shaderc_optimization_level_performance);

            shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(m_Source, ShadercKindFromShaderType(m_Type), m_Path.c_str(), options);

            if(module.GetCompilationStatus() != shaderc_compilation_status_success) {
                SNOW_CORE_ERROR(module.GetErrorMessage());
            }

            const uint8_t* begin = (uint8_t*)module.cbegin();
            const uint8_t* end = (uint8_t*)module.cend();
            const ptrdiff_t size = end - begin;
            SNOW_CORE_TRACE("Size of shader {0}", size);

            m_RendererID = glCreateShader(GetShaderType(m_Type));
            glShaderBinary(1, &m_RendererID, GL_SHADER_BINARY_FORMAT_SPIR_V, (const void*)begin, size);
            glSpecializeShader(m_RendererID, "main", 0, nullptr, nullptr);
        }

        std::string OpenGLShader::ReadShaderFromFile(const std::string& path) {
            std::string result;
            std::ifstream inFile(path, std::ios::in | std::ios::binary);
            if(inFile.is_open()) {
                inFile.seekg(0, std::ios::end);
                result.resize(inFile.tellg());
                inFile.seekg(0, std::ios::beg);
                inFile.read(&result[0], result.size());
            }
            else {
                SNOW_CORE_ERROR("Could not read file. (Path):{0}", path);
            }
            inFile.close();
            return result;
        }

        GLenum OpenGLShader::GetShaderType(ShaderType type) {
            switch(type) {
            case ShaderType::Vertex:    return GL_VERTEX_SHADER;
            case ShaderType::Pixel:    return GL_FRAGMENT_SHADER;
            case ShaderType::Compute:   return GL_COMPUTE_SHADER;
            }
        }
    }
}