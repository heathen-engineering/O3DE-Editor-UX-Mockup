#pragma once

#include <functional>
#include <QString>
#include <QVector>

namespace Heathen
{
    class EditorWindow;

    struct PanelDescriptor
    {
        QString name;
        QString category = QStringLiteral("Other");
        std::function<EditorWindow*()> factory;

        enum class OpenIn { MostRecentContainer, NewWindow, NextTo };
        OpenIn openIn = OpenIn::MostRecentContainer;
        QString openNextTo;

        bool noClose = false;
        bool noPin   = false;
        QVector<QString> requiredPanels;
    };

    class EditorWindowRegistry
    {
    public:
        static void registerPanel(const PanelDescriptor& descriptor);
        static const QVector<PanelDescriptor>& getAllPanels();
        static QVector<const PanelDescriptor*> getPanelsInCategory(const QString& category);
        static EditorWindow* createPanel(const QString& name);
        static bool isRegistered(const QString& name);

    private:
        static QVector<PanelDescriptor>& storage();
    };

} // namespace Heathen
