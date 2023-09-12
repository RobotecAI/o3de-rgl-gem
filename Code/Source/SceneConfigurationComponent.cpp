#include <RGL/RGLBus.h>
#include <SceneConfigurationComponent.h>

namespace RGL
{
    void SceneConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<SceneConfiguration>()->Version(0)->Field(
                "SkinnedMeshUpdate", &SceneConfiguration::m_isSkinnedMeshUpdateEnabled);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                // clang-format off
                editContext->Class<SceneConfiguration>("RGL Scene Configuration", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &SceneConfiguration::m_isSkinnedMeshUpdateEnabled,
                        "Skinned Mesh Update",
                        "Should the Skinned Meshes be updated?");
                // clang-format on
            }
        }
    }

    void SceneConfigurationComponent::Reflect(AZ::ReflectContext* context)
    {
        SceneConfiguration::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<SceneConfigurationComponent, AZ::Component>()->Version(0)->Field(
                "Config", &SceneConfigurationComponent::m_config);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                // clang-format off
                editContext->Class<SceneConfigurationComponent>("RGL Scene Configuration", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "RGL")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZStd::vector<AZ::Crc32>({ AZ_CRC_CE("Level") }))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SceneConfigurationComponent::m_config, "Config", "");
                // clang-format on
            }
        }
    }

    void SceneConfigurationComponent::Activate()
    {
        RGLInterface::Get()->SetSceneConfiguration(m_config);
    }

    void SceneConfigurationComponent::Deactivate()
    {
    }
} // namespace RGL