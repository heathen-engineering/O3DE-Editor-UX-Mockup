
#pragma once

#include <Editor-Mock-up/Editor-Mock-upTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Editor_Mock_up
{
    class Editor_Mock_upRequests
    {
    public:
        AZ_RTTI(Editor_Mock_upRequests, Editor_Mock_upRequestsTypeId);
        virtual ~Editor_Mock_upRequests() = default;
        // Put your public methods here
    };

    class Editor_Mock_upBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using Editor_Mock_upRequestBus = AZ::EBus<Editor_Mock_upRequests, Editor_Mock_upBusTraits>;
    using Editor_Mock_upInterface = AZ::Interface<Editor_Mock_upRequests>;

} // namespace Editor_Mock_up
