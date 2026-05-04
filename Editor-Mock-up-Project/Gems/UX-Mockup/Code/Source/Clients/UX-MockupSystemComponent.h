
#pragma once

#include <AzCore/Component/Component.h>
#include <UX-Mockup/UX-MockupBus.h>

namespace UX_Mockup
{
    class UX_MockupSystemComponent
        : public AZ::Component
        , protected UX_MockupRequestBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(UX_MockupSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        UX_MockupSystemComponent();
        ~UX_MockupSystemComponent();

    protected:
        void Init() override;
        void Activate() override;
        void Deactivate() override;
    };

} // namespace UX_Mockup
