#pragma once

#include <UX-Mockup/PinSystem.h>

#include <QWidget>

#if !defined(Q_MOC_RUN)
#include <QVector>
#endif

class QPushButton;

namespace Heathen
{

class EditorWindow;
class EditorTabBar;

// The tab container that lives inside an EditorWindowContainer.
// Manages the ordered list of panels, the active panel, and the tab bar's
// auto-show/hide state. Pin requests are forwarded via panelPinRequested.
class EditorBody : public QWidget
{
    Q_OBJECT

public:
    explicit EditorBody(QWidget* parent = nullptr);

    void addPanel(EditorWindow* panel);
    void removePanel(EditorWindow* panel);
    void setActivePanel(EditorWindow* panel);
    void reorderPanel(EditorWindow* panel, int newIndex);

    EditorWindow* activePanel() const;
    const QVector<EditorWindow*>& panels() const;

    // Count of tabbed panels. Drives tab bar show/hide.
    int visiblePanelCount() const;

    // Returns the insertion index (0-based) for a drop at globalPos within the tab bar.
    int tabInsertionIndexAt(QPoint globalPos) const;

signals:
    void becameEmpty();
    void panelPinRequested(Heathen::EditorWindow* panel, Heathen::PinPosition pos);

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void updateTabBarVisibility();
    void positionActivePanel();
    void positionGrabHandle();

    EditorTabBar* m_tabBar     = nullptr;
    QWidget*      m_content    = nullptr;
    QPushButton*  m_grabHandle = nullptr;

    QVector<EditorWindow*> m_panels;
    EditorWindow* m_activePanel      = nullptr;
    bool          m_tabBarVisible    = false;
    bool          m_tabBarTemporary  = false;
};

} // namespace Heathen
