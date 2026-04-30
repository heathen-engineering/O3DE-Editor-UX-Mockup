
#pragma once

#include <Clients/UX-MockupSystemComponent.h>

#include <AzToolsFramework/Entity/EditorEntityContextBus.h>

namespace UX_Mockup
{
    /// System component for UX_Mockup editor
    class UX_MockupEditorSystemComponent
        : public UX_MockupSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = UX_MockupSystemComponent;
    public:
        AZ_COMPONENT_DECL(UX_MockupEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        UX_MockupEditorSystemComponent();
        ~UX_MockupEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        // AzToolsFramework::EditorEventsBus overrides ...
        void NotifyRegisterViews() override;
    };
} // namespace UX_Mockup
