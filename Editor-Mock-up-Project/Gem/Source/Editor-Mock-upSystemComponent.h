
#pragma once

#include <AzCore/Component/Component.h>

#include <Editor-Mock-up/Editor-Mock-upBus.h>

namespace Editor_Mock_up
{
    class Editor_Mock_upSystemComponent
        : public AZ::Component
        , protected Editor_Mock_upRequestBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(Editor_Mock_upSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        Editor_Mock_upSystemComponent();
        ~Editor_Mock_upSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // Editor_Mock_upRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
