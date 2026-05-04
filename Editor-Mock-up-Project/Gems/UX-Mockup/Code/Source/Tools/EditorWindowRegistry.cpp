#include <UX-Mockup/EditorWindowRegistry.h>
#include <UX-Mockup/EditorWindow.h>

#include <AzCore/Debug/Trace.h>

namespace Heathen
{
    QVector<PanelDescriptor>& EditorWindowRegistry::storage()
    {
        static QVector<PanelDescriptor> s_panels;
        return s_panels;
    }

    void EditorWindowRegistry::registerPanel(const PanelDescriptor& descriptor)
    {
        if (descriptor.name.isEmpty())
        {
            AZ_Warning("EditorWindowRegistry", false, "Panel registration skipped: name is empty.");
            return;
        }
        if (!descriptor.factory)
        {
            AZ_Warning("EditorWindowRegistry", false, "Panel '%s' registration skipped: factory is null.",
                descriptor.name.toUtf8().constData());
            return;
        }
        if (isRegistered(descriptor.name))
        {
            AZ_Warning("EditorWindowRegistry", false, "Panel '%s' is already registered — skipping duplicate.",
                descriptor.name.toUtf8().constData());
            return;
        }
        storage().append(descriptor);
    }

    const QVector<PanelDescriptor>& EditorWindowRegistry::getAllPanels()
    {
        return storage();
    }

    QVector<const PanelDescriptor*> EditorWindowRegistry::getPanelsInCategory(const QString& category)
    {
        QVector<const PanelDescriptor*> result;
        for (const PanelDescriptor& desc : storage())
        {
            if (desc.category == category)
                result.append(&desc);
        }
        return result;
    }

    EditorWindow* EditorWindowRegistry::createPanel(const QString& name)
    {
        for (const PanelDescriptor& desc : storage())
        {
            if (desc.name == name)
            {
                EditorWindow* panel = desc.factory();
                panel->buildContent();
                return panel;
            }
        }
        AZ_Warning("EditorWindowRegistry", false, "createPanel: no panel named '%s' is registered.",
            name.toUtf8().constData());
        return nullptr;
    }

    bool EditorWindowRegistry::isRegistered(const QString& name)
    {
        for (const PanelDescriptor& desc : storage())
        {
            if (desc.name == name)
                return true;
        }
        return false;
    }

} // namespace Heathen
