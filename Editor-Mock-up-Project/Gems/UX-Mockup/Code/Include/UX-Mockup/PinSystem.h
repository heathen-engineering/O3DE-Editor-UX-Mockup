#pragma once

#include <QWidget>

#if !defined(Q_MOC_RUN)
#include <functional>
#include <QVector>
#endif

class QLabel;
class QPushButton;
class QVBoxLayout;

namespace Heathen
{

class EditorWindow;

enum class PinPosition { TopLeft, TopRight, BottomLeft, BottomRight };

// One clickable icon in a PinBar representing a single pinned panel.
class PinIconButton : public QWidget
{
public:
    explicit PinIconButton(EditorWindow* panel, QWidget* parent = nullptr);
    EditorWindow* panel() const;
    void setHighlighted(bool on);

    std::function<void()> onClick;
    std::function<void()> onUnpinRequested;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    EditorWindow* m_panel       = nullptr;
    bool          m_highlighted = false;
    bool          m_hovered     = false;
};

// 20 px-wide vertical strip on the left or right window edge.
class PinBar : public QWidget
{
    Q_OBJECT
public:
    explicit PinBar(bool isLeft, QWidget* parent = nullptr);

    void addButton(EditorWindow* panel, bool topSection,
                   std::function<void(EditorWindow*)> onActivate,
                   std::function<void(EditorWindow*)> onUnpin);
    void removeButton(EditorWindow* panel);
    void setHighlighted(EditorWindow* panel, bool on);

private:
    QVBoxLayout*            m_topLayout = nullptr;
    QVBoxLayout*            m_botLayout = nullptr;
    QVector<PinIconButton*> m_buttons;
};

// Wrapper widget placed inside the pin layout splitter (pinned-open) or shown
// as an overlay floating over the main window content.
// Overlay mode supports edge-drag resizing.
class PinPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PinPanel(PinPosition pos, QWidget* parent = nullptr);
    ~PinPanel() override;

    void addPanel(EditorWindow* panel);
    void removePanel(EditorWindow* panel);
    void setActivePanel(EditorWindow* panel);

    EditorWindow*                  activePanel() const;
    const QVector<EditorWindow*>&  panels()      const;
    bool                           isEmpty()     const;

    PinPosition position()     const;
    bool        isPinnedOpen() const;
    void        setPinnedOpen(bool open);

    // Called by EditorWindowContainer when switching to/from overlay.
    void setOverlayMode(bool overlay);

    std::function<void()> onPinToggled;
    std::function<void()> onUnpin;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void buildUI();
    void positionActivePanel();
    void updateTitle();
    bool inResizeZone(QPoint localPos) const;
    Qt::CursorShape resizeCursor() const;

    PinPosition            m_pos;
    bool                   m_pinnedOpen  = false;
    bool                   m_overlayMode = false;
    QVector<EditorWindow*> m_panels;
    EditorWindow*          m_activePanel = nullptr;

    QLabel*      m_titleLabel  = nullptr;
    QPushButton* m_pinBtn      = nullptr;
    QWidget*     m_contentArea = nullptr;

    // Overlay resize state
    bool   m_resizing        = false;
    QPoint m_resizeAnchor;
    QRect  m_resizeStartRect;
};

} // namespace Heathen
