
#include <UX-Mockup/UX-MockupTypeIds.h>
#include <UX-MockupModuleInterface.h>
#include "UX-MockupEditorSystemComponent.h"

void InitUX_MockupResources()
{
    // We must register our Qt resources (.qrc file) since this is being loaded from a separate module (gem)
    Q_INIT_RESOURCE(UX_Mockup);
}

namespace UX_Mockup
{
    class UX_MockupEditorModule
        : public UX_MockupModuleInterface
    {
    public:
        AZ_RTTI(UX_MockupEditorModule, UX_MockupEditorModuleTypeId, UX_MockupModuleInterface);
        AZ_CLASS_ALLOCATOR(UX_MockupEditorModule, AZ::SystemAllocator);

        UX_MockupEditorModule()
        {
            InitUX_MockupResources();

            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                UX_MockupEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<UX_MockupEditorSystemComponent>(),
            };
        }
    };
}// namespace UX_Mockup

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), UX_Mockup::UX_MockupEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_UX_Mockup_Editor, UX_Mockup::UX_MockupEditorModule)
#endif
