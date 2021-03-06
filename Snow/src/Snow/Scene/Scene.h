#pragma once

#include "Snow/Core/Ref.h"
#include "Snow/Render/EditorCamera.h"
#include "Snow/Render/API/Texture.h"

#include "Snow/Core/Timestep.h"
#include "Snow/Core/UUID.h"

#include <entt.hpp>

#include <glm/glm.hpp>

#include <box2d/b2_world.h>

namespace Snow {

    class Entity;
    using EntityMap = std::unordered_map<UUID, Entity>;

    struct Light {
        glm::vec3 Direction = { 0.0f, 0.0f, 0.0f };
        float padding = 0.0f;
        glm::vec3 Radiance = { 0.0f, 0.0f, 0.0f };

        float Multiplier = 1.0f;
    };

    class Scene : public RefCounted {
    public:

        Scene(const std::string& name = "Unnamed Scene");
        ~Scene();

        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithID(UUID uuid, const std::string& name = std::string(), bool runtimeMap = false);
        void DestroyEntity(Entity entity);

        template<typename T>
        auto GetAllEntitiesWith() {
            return m_Registry.view<T>();
        }

        void OnRuntimeStart();
        void OnRuntimeStop();

        void OnUpdate(Timestep ts);
        void OnRenderRuntime(Timestep ts);
        void OnRenderEditor(Timestep ts, Render::EditorCamera& editorCamera);

        void OnViewportResize(uint32_t width, uint32_t height);

        Entity FindEntityByTag(const std::string& tag);
        void CopyScene(Ref<Scene> scene);

        Light& GetLight() { return m_Light; }
        const Light& GetLight() const { return m_Light; }

        b2World* GetPhysicsWorld() const { return m_PhysicsWorld; }
        
        Entity GetMainCamera();

        const EntityMap& GetEntityMap() const { return m_EntityIDMap; }

        UUID GetUUID() const { return m_SceneID; }
        static Ref<Scene> GetScene(UUID uuid);
    private:
        UUID m_SceneID;
        entt::entity m_SceneEntity;
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

        EntityMap m_EntityIDMap;

        Light m_Light;
        float m_LightMultiplier = 0.3f;

        Ref<Render::API::TextureCube> m_EnvMap;
        Ref<Render::API::Texture2D> m_BRDFLUT;

        bool m_IsPlaying;

        std::string m_Name;

        b2Vec2 m_Gravity = b2Vec2(0.0f, -1.0f);
        b2World* m_PhysicsWorld;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneRenderer;
        friend class SceneHierarchyPanel;

        friend void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity);
        friend void OnScriptComponentDestroy(entt::registry& registry, entt::entity entity);
    };

}