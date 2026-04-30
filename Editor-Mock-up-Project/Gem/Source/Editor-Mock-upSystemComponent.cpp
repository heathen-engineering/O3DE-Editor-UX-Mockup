
#include <AzCore/Serialization/SerializeContext.h>

#include "Editor-Mock-upSystemComponent.h"

#include <Editor-Mock-up/Editor-Mock-upTypeIds.h>

namespace Editor_Mock_up
{
    AZ_COMPONENT_IMPL(Editor_Mock_upSystemComponent, "Editor_Mock_upSystemComponent",
        Editor_Mock_upSystemComponentTypeId);

    void Editor_Mock_upSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<Editor_Mock_upSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void Editor_Mock_upSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("Editor_Mock_upService"));
    }

    void Editor_Mock_upSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("Editor_Mock_upService"));
    }

    void Editor_Mock_upSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void Editor_Mock_upSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    Editor_Mock_upSystemComponent::Editor_Mock_upSystemComponent()
    {
        if (Editor_Mock_upInterface::Get() == nullptr)
        {
            Editor_Mock_upInterface::Register(this);
        }
    }

    Editor_Mock_upSystemComponent::~Editor_Mock_upSystemComponent()
    {
        if (Editor_Mock_upInterface::Get() == this)
        {
            Editor_Mock_upInterface::Unregister(this);
        }
    }

    void Editor_Mock_upSystemComponent::Init()
    {
    }

    void Editor_Mock_upSystemComponent::Activate()
    {
        Editor_Mock_upRequestBus::Handler::BusConnect();
    }

    void Editor_Mock_upSystemComponent::Deactivate()
    {
        Editor_Mock_upRequestBus::Handler::BusDisconnect();
    }
}
