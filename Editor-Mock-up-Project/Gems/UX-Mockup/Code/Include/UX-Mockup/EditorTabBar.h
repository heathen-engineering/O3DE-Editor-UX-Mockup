#pragma once

#include <UX-Mockup/PinSystem.h>

#include <QWidget>

#if !defined(Q_MOC_RUN)
#include <functional>
#include <QVector>
#endif

class QLabel;
class QPushButton;
class QPropertyAnimation;

namespace Heathen
{

class EditorWindow;

// One tab in the tab bar. Communicates via std::function callbacks set by EditorTabBar.
class EditorTab : public QWidget
{
public:
    explicit EditorTab(EditorWindow* panel, QWidget* parent = nullptr);

    EditorWindow* panel() const;
    void setActive(bool active);
    bool isActive() const;

    std::function<void()>               onActivated;
    std::function<void()>               onCloseRequested;
    std::function<void(PinPosition)>    onPinRequested;
    std::function<void(QPoint)>         onDragStarted;

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void applyStyle();

    EditorWindow* m_panel       = nullptr;
    QLabel*       m_label       = nullptr;
    QPushButton*  m_closeBtn    = nullptr;
    bool          m_active      = false;
    bool          m_hovered     = false;
    bool          m_dragging    = false;
    QPoint        m_dragStartPos;
};

// Wrapping tab bar that grows in height rather than scrolling.
// Animated show/hide via the animatedHeight Q_PROPERTY.
class EditorTabBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int animatedHeight READ animatedHeight WRITE setAnimatedHeight)

public:
    explicit EditorTabBar(QWidget* parent = nullptr);

    void addTab(EditorWindow* panel);
    void removeTab(EditorWindow* panel);
    void setActiveTab(EditorWindow* panel);
    void reorderTab(EditorWindow* panel, int insertBefore);

    // Returns the tab insertion index (0..count) for a drop at globalPos.
    int tabInsertionIndexAt(QPoint globalPos) const;

    void showAnimated(bool temporary = false);
    void hideAnimated();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    int  animatedHeight() const;
    void setAnimatedHeight(int h);

signals:
    void tabActivated(Heathen::EditorWindow* panel);
    void tabCloseRequested(Heathen::EditorWindow* panel);
    void tabPinRequested(Heathen::EditorWindow* panel, Heathen::PinPosition pos);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void relayout();
    int  computeNaturalHeight() const;
    int  tabWidth(const EditorTab* tab) const;
    void stopAnimation();

    QVector<EditorTab*>  m_tabs;
    QPropertyAnimation*  m_animation     = nullptr;
    int                  m_animatedHeight = 0;
};

} // namespace Heathen
