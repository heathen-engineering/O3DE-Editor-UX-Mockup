
#include "UX-MockupModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <UX-Mockup/UX-MockupTypeIds.h>

#include <Clients/UX-MockupSystemComponent.h>

namespace UX_Mockup
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(UX_MockupModuleInterface,
        "UX_MockupModuleInterface", UX_MockupModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(UX_MockupModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(UX_MockupModuleInterface, AZ::SystemAllocator);

    UX_MockupModuleInterface::UX_MockupModuleInterface()
    {
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
        m_descriptors.insert(m_descriptors.end(), {
            UX_MockupSystemComponent::CreateDescriptor(),
            });
    }

    AZ::ComponentTypeList UX_MockupModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<UX_MockupSystemComponent>(),
        };
    }
} // namespace UX_Mockup
