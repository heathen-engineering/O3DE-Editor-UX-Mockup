#pragma once

#include <UX-Mockup/PinSystem.h>

#include <QWidget>

#if !defined(Q_MOC_RUN)
#include <QJsonObject>
#include <QVector>
#include <QString>
#endif

class QLabel;
class QMenuBar;
class QSplitter;

namespace Heathen
{

class EditorWindow;
class EditorBody;

// A real OS window (top-level QWidget) that holds EditorWindow panels.
// The main window variant carries the menu bar, menu-driven panel list, and the
// pin system (PinBars + PinPanels). Torn-off windows are simpler containers
// without pins. Every container self-registers on construction.
class EditorWindowContainer : public QWidget
{
    Q_OBJECT

public:
    explicit EditorWindowContainer(bool isMainWindow = false, QWidget* parent = nullptr);
    ~EditorWindowContainer() override;

    bool isMainWindow() const;

    // The primary dock body for this container.
    EditorBody* body() const;

    // All EditorBody instances reachable from this container's layout tree.
    QVector<EditorBody*> allBodies() const;

    bool hasRoom() const;

    void addPanel(EditorWindow* panel);
    void removePanel(EditorWindow* panel);
    const QVector<EditorWindow*>& panels() const;

    // Split 'target' body at the given edge, returning the new empty body.
    EditorBody* splitBody(EditorBody* target, Qt::Edge edge);

    // Pin a panel (removed from its current body) to the given corner of the
    // main window. Delegates to s_mainWindow->pinPanel() if called on a secondary
    // container.
    void pinPanel(EditorWindow* panel, PinPosition pos);

    // Move a pinned panel back to the dock system (most-recent body).
    void unpinPanel(EditorWindow* panel);

    static EditorWindowContainer* containerForBody(EditorBody* body);
    static void openPanel(const QString& name);
    static EditorWindowContainer* mostRecentWithRoom();
    static EditorWindowContainer* containerHolding(const QString& panelName);
    static const QVector<EditorWindowContainer*>& allContainers();
    static EditorWindowContainer* mainWindow();
    static EditorWindowContainer* createMainWindow();

    static void saveLayoutToFile();
    static void loadLayoutFromFile();

protected:
    void changeEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void relocateNoClosePanels();
    void wireBody(EditorBody* body);
    void handleBodyEmpty(EditorBody* body);
    void applyLayoutJson(const QJsonObject& windowObj);

    // Pin system helpers (main window only).
    void activatePanelAtCorner(PinPosition pos, EditorWindow* panel);
    void showOverlay(int cornerIdx);
    void hideOverlay(int cornerIdx);
    void moveToPinnedOpen(int cornerIdx);
    void moveToCollapsed(int cornerIdx);
    QRect overlayRect(PinPosition pos) const;
    void ensureBottomSplitter();
    void maybeRemoveBottomSplitter();
    QRect workAreaRect() const;

    bool        m_isMainWindow = false;
    QMenuBar*   m_menuBar      = nullptr;
    EditorBody* m_body         = nullptr;
    QLabel*     m_statusLabel  = nullptr;

    // Pin system layout (main window only).
    PinBar*    m_leftPinBar        = nullptr;
    PinBar*    m_rightPinBar       = nullptr;
    QSplitter* m_workVSplitter     = nullptr;
    QSplitter* m_topHSplitter      = nullptr;
    QSplitter* m_bottomHSplitter   = nullptr;

    // Per-corner state.
    enum class PinState { Collapsed, Overlay, PinnedOpen };
    PinPanel* m_pinPanels[4] = { nullptr, nullptr, nullptr, nullptr };
    PinState  m_pinStates[4] = { PinState::Collapsed, PinState::Collapsed,
                                  PinState::Collapsed, PinState::Collapsed };

    static QVector<EditorWindowContainer*>& registry();
    static EditorWindowContainer* s_mostRecent;
    static EditorWindowContainer* s_mainWindow;
};

} // namespace Heathen
