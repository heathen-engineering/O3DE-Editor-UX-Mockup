#include <UX-Mockup/EditorWindow.h>
#include <UX-Mockup/EditorWindowRegistry.h>

#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QShowEvent>
#include <QStyle>

namespace Heathen
{
    EditorWindow::EditorWindow(const QString& panelName, QWidget* parent)
        : QWidget(parent)
        , m_panelName(panelName)
    {
    }

    QIcon EditorWindow::tabIcon() const
    {
        return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    }

    void EditorWindow::setBackgroundColor(const QColor& color)
    {
        m_bgColor = color;
        update();
    }

    void EditorWindow::paintEvent(QPaintEvent*)
    {
        QPainter painter(this);
        painter.fillRect(rect(), m_bgColor);
    }

    void EditorWindow::showEvent(QShowEvent* event)
    {
        QWidget::showEvent(event);
        update();
    }

    const QString& EditorWindow::panelName() const
    {
        return m_panelName;
    }

    bool EditorWindow::isNoClose() const
    {
        for (const PanelDescriptor& desc : EditorWindowRegistry::getAllPanels())
        {
            if (desc.name == m_panelName)
                return desc.noClose;
        }
        return false;
    }

    bool EditorWindow::isNoPin() const
    {
        for (const PanelDescriptor& desc : EditorWindowRegistry::getAllPanels())
        {
            if (desc.name == m_panelName)
                return desc.noPin;
        }
        return false;
    }

    QVector<QString> EditorWindow::requiredPanels() const
    {
        for (const PanelDescriptor& desc : EditorWindowRegistry::getAllPanels())
        {
            if (desc.name == m_panelName)
                return desc.requiredPanels;
        }
        return {};
    }

} // namespace Heathen
