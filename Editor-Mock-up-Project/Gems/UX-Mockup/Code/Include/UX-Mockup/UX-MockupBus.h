
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace UX_Mockup
{
    class UX_MockupRequests
    {
    public:
        AZ_RTTI(UX_MockupRequests, "{8F32AC83-AF8B-41D5-81F7-672056D4A3C5}");
        virtual ~UX_MockupRequests() = default;
        // Put your public methods here
    };
    
    class UX_MockupBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using UX_MockupRequestBus = AZ::EBus<UX_MockupRequests, UX_MockupBusTraits>;
    using UX_MockupInterface = AZ::Interface<UX_MockupRequests>;

} // namespace UX_Mockup
