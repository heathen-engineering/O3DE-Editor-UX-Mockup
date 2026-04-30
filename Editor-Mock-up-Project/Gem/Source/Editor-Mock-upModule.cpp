
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include "Editor-Mock-upSystemComponent.h"

#include <Editor-Mock-up/Editor-Mock-upTypeIds.h>

namespace Editor_Mock_up
{
    class Editor_Mock_upModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(Editor_Mock_upModule, Editor_Mock_upModuleTypeId, AZ::Module);
        AZ_CLASS_ALLOCATOR(Editor_Mock_upModule, AZ::SystemAllocator);

        Editor_Mock_upModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                Editor_Mock_upSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<Editor_Mock_upSystemComponent>(),
            };
        }
    };
}// namespace Editor_Mock_up

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), Editor_Mock_up::Editor_Mock_upModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_Editor_Mock_up, Editor_Mock_up::Editor_Mock_upModule)
#endif
