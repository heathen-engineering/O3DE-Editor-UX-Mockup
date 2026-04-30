
#include "UX-MockupSystemComponent.h"

#include <UX-Mockup/UX-MockupTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace UX_Mockup
{
    AZ_COMPONENT_IMPL(UX_MockupSystemComponent, "UX_MockupSystemComponent",
        UX_MockupSystemComponentTypeId);

    void UX_MockupSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UX_MockupSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void UX_MockupSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("UX_MockupService"));
    }

    void UX_MockupSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("UX_MockupService"));
    }

    void UX_MockupSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void UX_MockupSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    UX_MockupSystemComponent::UX_MockupSystemComponent()
    {
        if (UX_MockupInterface::Get() == nullptr)
        {
            UX_MockupInterface::Register(this);
        }
    }

    UX_MockupSystemComponent::~UX_MockupSystemComponent()
    {
        if (UX_MockupInterface::Get() == this)
        {
            UX_MockupInterface::Unregister(this);
        }
    }

    void UX_MockupSystemComponent::Init()
    {
    }

    void UX_MockupSystemComponent::Activate()
    {
        UX_MockupRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void UX_MockupSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        UX_MockupRequestBus::Handler::BusDisconnect();
    }

    void UX_MockupSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace UX_Mockup
