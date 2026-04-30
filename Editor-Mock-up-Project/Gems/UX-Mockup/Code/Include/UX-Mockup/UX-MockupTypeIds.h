
#pragma once

namespace UX_Mockup
{
    // System Component TypeIds
    inline constexpr const char* UX_MockupSystemComponentTypeId = "{BF596C9A-C6A9-4181-A9E7-EF7F0F06A90C}";
    inline constexpr const char* UX_MockupEditorSystemComponentTypeId = "{465B9C92-27E8-45E8-801D-70CF5F2F9912}";

    // Module derived classes TypeIds
    inline constexpr const char* UX_MockupModuleInterfaceTypeId = "{62B990D2-DDA2-476C-96FB-FDC3DA1F5D0A}";
    inline constexpr const char* UX_MockupModuleTypeId = "{77945B7B-1B8B-4EAB-9258-2311D7C28F99}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* UX_MockupEditorModuleTypeId = UX_MockupModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* UX_MockupRequestsTypeId = "{24288CFF-9340-4417-B466-75F135C111D8}";
} // namespace UX_Mockup
