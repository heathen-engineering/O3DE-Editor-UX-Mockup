#include <UX-Mockup/EditorBody.h>
#include <UX-Mockup/EditorTabBar.h>
#include <UX-Mockup/EditorWindow.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>

namespace {
constexpr int k_grabZoneSize   = 40;
constexpr int k_grabHandleSize = 20;
}

namespace Heathen
{

EditorBody::EditorBody(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("EditorBody");
    setMinimumSize(50, 50);

    m_tabBar = new EditorTabBar(this);

    connect(m_tabBar, &EditorTabBar::tabActivated, this, [this](EditorWindow* panel)
    {
        setActivePanel(panel);
    });

    connect(m_tabBar, &EditorTabBar::tabCloseRequested, this, [this](EditorWindow* panel)
    {
        QTimer::singleShot(0, this, [this, panel]()
        {
            removePanel(panel);
            delete panel;
        });
    });

    connect(m_tabBar, &EditorTabBar::tabPinRequested, this,
            [this](EditorWindow* panel, PinPosition pos)
    {
        QTimer::singleShot(0, this, [this, panel, pos]()
        {
            emit panelPinRequested(panel, pos);
        });
    });

    m_content = new QWidget(this);
    m_content->setObjectName("ContentArea");
    m_content->setStyleSheet("QWidget#ContentArea { background-color: #282828; }");
    m_content->installEventFilter(this);

    m_grabHandle = new QPushButton(QStringLiteral("⋮"), m_content);
    m_grabHandle->setObjectName("GrabHandle");
    m_grabHandle->setFixedSize(k_grabHandleSize, k_grabHandleSize);
    m_grabHandle->setToolTip(tr("Show tab bar"));
    m_grabHandle->setStyleSheet(
        "QPushButton#GrabHandle {"
        "  background-color: #555; color: #ccc; border: none; border-radius: 3px; font-size: 10px;"
        "}"
        "QPushButton#GrabHandle:hover { background-color: #007acc; color: #fff; }");
    m_grabHandle->move(4, 4);
    m_grabHandle->hide();
    m_grabHandle->raise();

    connect(m_grabHandle, &QPushButton::clicked, this, [this]()
    {
        m_grabHandle->hide();
        m_tabBarTemporary = true;
        m_tabBar->showAnimated(true);
    });

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_tabBar);
    mainLayout->addWidget(m_content, 1);
    setLayout(mainLayout);

    qApp->installEventFilter(this);
}

void EditorBody::addPanel(EditorWindow* panel)
{
    if (!panel || m_panels.contains(panel))
        return;
    m_panels.append(panel);
    panel->setParent(m_content);
    panel->hide();
    m_tabBar->addTab(panel);
    updateTabBarVisibility();
    setActivePanel(panel);
}

void EditorBody::removePanel(EditorWindow* panel)
{
    if (!panel || !m_panels.contains(panel))
        return;

    const bool wasActive = (panel == m_activePanel);
    m_panels.removeAll(panel);
    m_tabBar->removeTab(panel);
    panel->hide();
    panel->setParent(nullptr);

    if (wasActive)
    {
        m_activePanel = nullptr;
        if (!m_panels.isEmpty())
            setActivePanel(m_panels.last());
    }

    updateTabBarVisibility();

    if (m_panels.isEmpty())
        emit becameEmpty();
}

void EditorBody::setActivePanel(EditorWindow* panel)
{
    if (!panel || !m_panels.contains(panel))
        return;
    if (m_activePanel)
        m_activePanel->hide();
    m_activePanel = panel;
    panel->setParent(m_content);
    panel->show();
    positionActivePanel();
    m_tabBar->setActiveTab(panel);
}

EditorWindow* EditorBody::activePanel() const { return m_activePanel; }

const QVector<EditorWindow*>& EditorBody::panels() const { return m_panels; }

int EditorBody::visiblePanelCount() const { return m_panels.size(); }

void EditorBody::updateTabBarVisibility()
{
    const bool shouldShow = visiblePanelCount() >= 2;
    if (shouldShow == m_tabBarVisible)
        return;
    m_tabBarVisible = shouldShow;
    if (m_tabBarVisible)
    {
        m_grabHandle->hide();
        m_tabBar->showAnimated();
    }
    else
    {
        m_tabBar->hideAnimated();
    }
}

void EditorBody::positionActivePanel()
{
    if (m_activePanel)
        m_activePanel->setGeometry(m_content->rect());
}

void EditorBody::positionGrabHandle()
{
    m_grabHandle->move(4, 4);
    m_grabHandle->raise();
}

void EditorBody::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    positionActivePanel();
    positionGrabHandle();
}

bool EditorBody::eventFilter(QObject* watched, QEvent* event)
{
    // Global mouse tracking for the grab-zone handle.
    if (event->type() == QEvent::MouseMove)
    {
        const QPoint gp = static_cast<QMouseEvent*>(event)->globalPos();
        if (!m_tabBarVisible)
        {
            const QPoint cp = m_content->mapFromGlobal(gp);
            if (m_content->rect().contains(cp))
            {
                const bool inZone = cp.x() < k_grabZoneSize && cp.y() < k_grabZoneSize;
                m_grabHandle->setVisible(inZone);
                if (inZone)
                    m_grabHandle->raise();
            }
            else
            {
                m_grabHandle->hide();
            }
        }
    }

    // Auto-hide the temporarily shown tab bar on any click outside it.
    if (event->type() == QEvent::MouseButtonPress && m_tabBarTemporary)
    {
        const QPoint gp = static_cast<QMouseEvent*>(event)->globalPos();
        const QPoint lp = m_tabBar->mapFromGlobal(gp);
        if (!m_tabBar->rect().contains(lp))
        {
            m_tabBarTemporary = false;
            m_tabBar->hideAnimated();
        }
    }

    (void)watched;
    return QWidget::eventFilter(watched, event);
}

void EditorBody::reorderPanel(EditorWindow* panel, int newIndex)
{
    const int fromIdx = m_panels.indexOf(panel);
    if (fromIdx < 0) return;
    m_panels.removeAt(fromIdx);
    newIndex = qBound(0, newIndex, m_panels.size());
    m_panels.insert(newIndex, panel);
    m_tabBar->reorderTab(panel, newIndex);
}

int EditorBody::tabInsertionIndexAt(QPoint globalPos) const
{
    return m_tabBar->tabInsertionIndexAt(globalPos);
}

} // namespace Heathen
