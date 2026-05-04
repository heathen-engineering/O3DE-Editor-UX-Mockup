#pragma once

#include <QWidget>

#if !defined(Q_MOC_RUN)
#include <QColor>
#include <QIcon>
#include <QString>
#include <QVector>
#endif

namespace Heathen
{
    // Abstract base class for every dockable panel in the workspace.
    // Subclasses implement buildContent() to populate their widget area.
    // Do NOT call buildContent() from a subclass constructor —
    // EditorWindowRegistry::createPanel() calls it after full construction.
    class EditorWindow : public QWidget
    {
        Q_OBJECT

    public:
        explicit EditorWindow(const QString& panelName, QWidget* parent = nullptr);

        // The registered name used to identify this panel type (e.g. "SceneView").
        const QString& panelName() const;

        // True if this panel cannot be closed — only moved or pinned.
        // Derived from the PanelDescriptor registered for this panel name.
        bool isNoClose() const;

        // True if this panel cannot be pinned to a corner — must stay dockable.
        bool isNoPin() const;

        // Panel names that must be open alongside this one.
        // Derived from the PanelDescriptor registered for this panel name.
        QVector<QString> requiredPanels() const;

        // Called once by EditorWindowRegistry::createPanel() after construction.
        // Subclasses add their child widgets and layouts here.
        virtual void buildContent() = 0;

        // Set the solid fill color drawn behind all child widgets.
        void setBackgroundColor(const QColor& color);

        // Icon shown in the tab for this panel. Override to provide a panel-specific icon.
        virtual QIcon tabIcon() const;

    protected:
        void paintEvent(QPaintEvent* event) override;
        void showEvent(QShowEvent* event) override;

    private:
        QString m_panelName;
        QColor  m_bgColor { "#252526" };
    };

} // namespace Heathen
