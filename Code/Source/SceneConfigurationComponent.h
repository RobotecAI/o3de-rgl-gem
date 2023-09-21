#pragma once

#include <AzCore/Component/Component.h>

namespace RGL
{
    //! Structure used to describe all global scene parameters.
    struct SceneConfiguration
    {
        AZ_TYPE_INFO(SceneConfiguration, "{7e55de90-e26c-4567-9e06-822c6ce62b9c}");
        static void Reflect(AZ::ReflectContext* context);

        bool m_isSkinnedMeshUpdateEnabled{ true }; //!< If set to true, all skinned meshes will be updated. Otherwise they will remain unchanged.
    };

    class SceneConfigurationComponent : public AZ::Component
    {
    public:
        AZ_COMPONENT(SceneConfigurationComponent, "{18c28e89-27f0-4230-936e-d7858b39d53f}", AZ::Component);

        SceneConfigurationComponent() = default;
        ~SceneConfigurationComponent() override = default;

        static void Reflect(AZ::ReflectContext* context);

        // clang-format off
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required) {}
        // clang-format on

        // AZ::Component overrides
        void Activate() override;
        void Deactivate() override;

    private:
        SceneConfiguration m_config;
    };
} // namespace RGL
