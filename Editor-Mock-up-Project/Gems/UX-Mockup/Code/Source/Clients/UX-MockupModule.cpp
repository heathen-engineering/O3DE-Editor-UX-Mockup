
#include <UX-Mockup/UX-MockupTypeIds.h>
#include <UX-MockupModuleInterface.h>
#include "UX-MockupSystemComponent.h"

namespace UX_Mockup
{
    class UX_MockupModule
        : public UX_MockupModuleInterface
    {
    public:
        AZ_RTTI(UX_MockupModule, UX_MockupModuleTypeId, UX_MockupModuleInterface);
        AZ_CLASS_ALLOCATOR(UX_MockupModule, AZ::SystemAllocator);
    };
}// namespace UX_Mockup

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), UX_Mockup::UX_MockupModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_UX_Mockup, UX_Mockup::UX_MockupModule)
#endif
