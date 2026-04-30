
#include <AzCore/Serialization/SerializeContext.h>

#include <AzToolsFramework/API/ViewPaneOptions.h>

#include "UX-MockupWidget.h"
#include "UX-MockupEditorSystemComponent.h"

#include <UX-Mockup/UX-MockupTypeIds.h>

namespace UX_Mockup
{
    AZ_COMPONENT_IMPL(UX_MockupEditorSystemComponent, "UX_MockupEditorSystemComponent",
        UX_MockupEditorSystemComponentTypeId, BaseSystemComponent);

    void UX_MockupEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UX_MockupEditorSystemComponent, UX_MockupSystemComponent>()
                ->Version(0);
        }
    }

    UX_MockupEditorSystemComponent::UX_MockupEditorSystemComponent() = default;

    UX_MockupEditorSystemComponent::~UX_MockupEditorSystemComponent() = default;

    void UX_MockupEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("UX_MockupEditorService"));
    }

    void UX_MockupEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("UX_MockupEditorService"));
    }

    void UX_MockupEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void UX_MockupEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void UX_MockupEditorSystemComponent::Activate()
    {
        UX_MockupSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void UX_MockupEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        UX_MockupSystemComponent::Deactivate();
    }

    void UX_MockupEditorSystemComponent::NotifyRegisterViews()
    {
        AzToolsFramework::ViewPaneOptions options;
        options.paneRect = QRect(100, 100, 500, 400);
        options.showOnToolsToolbar = true;
        options.toolbarIcon = ":/UX-Mockup/toolbar_icon.svg";

        // Register our custom widget as a dockable tool with the Editor under an Examples sub-menu
        AzToolsFramework::RegisterViewPane<UX_MockupWidget>("UX-Mockup", "Examples", options);
    }

} // namespace UX_Mockup
