#include <UX-Mockup/EditorWindowContainer.h>
#include <UX-Mockup/EditorBody.h>
#include <UX-Mockup/EditorWindow.h>
#include <UX-Mockup/EditorWindowRegistry.h>
#include <UX-Mockup/PinSystem.h>

#include <AzCore/Debug/Trace.h>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenuBar>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QSizeGrip>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>

namespace {

constexpr int k_pinOverlayWidth  = 280;
constexpr int k_pinOverlayHeight = 260;

static void collectBodies(QWidget* w, QVector<Heathen::EditorBody*>& out)
{
    if (auto* body = qobject_cast<Heathen::EditorBody*>(w))
        out.append(body);
    else if (auto* sp = qobject_cast<QSplitter*>(w))
        for (int i = 0; i < sp->count(); ++i)
            collectBodies(sp->widget(i), out);
}

class PhantomPanel final : public Heathen::EditorWindow
{
public:
    explicit PhantomPanel(const QString& name) : Heathen::EditorWindow(name) {}
    void buildContent() override
    {
        auto* lbl = new QLabel(QStringLiteral("[ %1 ]").arg(panelName()), this);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("color:#888; font-style:italic;");
        auto* lay = new QVBoxLayout(this);
        lay->addWidget(lbl);
        setLayout(lay);
    }
};

static Heathen::EditorWindow* createPanelOrPhantom(const QString& name)
{
    Heathen::EditorWindow* w = Heathen::EditorWindowRegistry::createPanel(name);
    if (!w) { w = new PhantomPanel(name); w->buildContent(); }
    return w;
}

static QJsonObject serializeBodyTree(QWidget* w)
{
    if (auto* body = qobject_cast<Heathen::EditorBody*>(w))
    {
        QJsonObject obj;
        obj[QStringLiteral("type")] = QStringLiteral("body");
        QJsonArray panels;
        for (const Heathen::EditorWindow* p : body->panels())
            panels.append(p->panelName());
        obj[QStringLiteral("panels")] = panels;
        if (body->activePanel())
            obj[QStringLiteral("active")] = body->activePanel()->panelName();
        return obj;
    }
    if (auto* sp = qobject_cast<QSplitter*>(w))
    {
        QJsonObject obj;
        obj[QStringLiteral("type")] = QStringLiteral("splitter");
        obj[QStringLiteral("orientation")] =
            sp->orientation() == Qt::Horizontal ? QStringLiteral("h") : QStringLiteral("v");
        QJsonArray sizes;
        for (int s : sp->sizes()) sizes.append(s);
        obj[QStringLiteral("sizes")] = sizes;
        QJsonArray children;
        for (int i = 0; i < sp->count(); ++i)
            children.append(serializeBodyTree(sp->widget(i)));
        obj[QStringLiteral("children")] = children;
        return obj;
    }
    return {};
}

static QWidget* deserializeBodyTree(const QJsonObject& node)
{
    const QString type = node[QStringLiteral("type")].toString();
    if (type == QLatin1String("body"))
    {
        auto* body = new Heathen::EditorBody();
        for (const QJsonValue v : node[QStringLiteral("panels")].toArray())
        {
            const QString name = v.toString();
            if (!name.isEmpty()) body->addPanel(createPanelOrPhantom(name));
        }
        const QString active = node[QStringLiteral("active")].toString();
        if (!active.isEmpty())
            for (Heathen::EditorWindow* p : body->panels())
                if (p->panelName() == active) { body->setActivePanel(p); break; }
        return body;
    }
    if (type == QLatin1String("splitter"))
    {
        const bool horiz = node[QStringLiteral("orientation")].toString() == QLatin1String("h");
        auto* sp = new QSplitter(horiz ? Qt::Horizontal : Qt::Vertical);
        sp->setHandleWidth(4);
        sp->setChildrenCollapsible(false);
        sp->setStyleSheet("QSplitter::handle { background-color: #3c3c3c; }");
        for (const QJsonValue v : node[QStringLiteral("children")].toArray())
        {
            QWidget* child = deserializeBodyTree(v.toObject());
            if (child) sp->addWidget(child);
        }
        QList<int> sizes;
        for (const QJsonValue v : node[QStringLiteral("sizes")].toArray())
            sizes.append(v.toInt());
        if (!sizes.isEmpty()) sp->setSizes(sizes);
        return sp;
    }
    return nullptr;
}

static QSplitter* makeSplitter(Qt::Orientation o)
{
    auto* sp = new QSplitter(o);
    sp->setHandleWidth(4);
    sp->setChildrenCollapsible(false);
    sp->setStyleSheet("QSplitter::handle { background-color: #3c3c3c; }");
    return sp;
}

} // anonymous namespace

namespace Heathen
{

EditorWindowContainer* EditorWindowContainer::s_mostRecent = nullptr;
EditorWindowContainer* EditorWindowContainer::s_mainWindow = nullptr;

QVector<EditorWindowContainer*>& EditorWindowContainer::registry()
{
    static QVector<EditorWindowContainer*> s_containers;
    return s_containers;
}

EditorWindowContainer::EditorWindowContainer(bool isMainWindow, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , m_isMainWindow(isMainWindow)
{
    setWindowTitle(isMainWindow ? "O3DE Editor (UX Mockup)" : "O3DE Panel");
    setMinimumSize(320, 240);
    resize(isMainWindow ? QSize(1280, 720) : QSize(640, 480));

    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    if (isMainWindow)
    {
        m_menuBar = new QMenuBar(this);

        auto* fileMenu = m_menuBar->addMenu(tr("File"));
        fileMenu->addAction(tr("Save Layout..."), this, []() { saveLayoutToFile(); });
        fileMenu->addAction(tr("Load Layout..."), this, []() { loadLayoutFromFile(); });
        fileMenu->addSeparator();
        fileMenu->addAction(tr("Close Window"), this, [this]() { close(); });

        auto* panelsMenu = m_menuBar->addMenu(tr("Panels"));
        connect(panelsMenu, &QMenu::aboutToShow, this, [this, panelsMenu]()
        {
            panelsMenu->clear();
            qDeleteAll(panelsMenu->findChildren<QMenu*>(QString(), Qt::FindDirectChildrenOnly));

            QMenu* openMenu = panelsMenu->addMenu(tr("Open"));
            QVector<QString> cats;
            for (const PanelDescriptor& desc : EditorWindowRegistry::getAllPanels())
                if (!cats.contains(desc.category)) cats.append(desc.category);
            for (const QString& cat : cats)
            {
                QMenu* catMenu = openMenu->addMenu(cat);
                for (const PanelDescriptor* desc : EditorWindowRegistry::getPanelsInCategory(cat))
                {
                    const QString name = desc->name;
                    catMenu->addAction(name, [name]() { openPanel(name); });
                }
            }
            panelsMenu->addSeparator();
            for (const PanelDescriptor& desc : EditorWindowRegistry::getAllPanels())
            {
                const bool isOpen = (containerHolding(desc.name) != nullptr);
                QAction* act = panelsMenu->addAction(desc.name);
                act->setCheckable(true);
                act->setChecked(isOpen);
                const QString name = desc.name;
                connect(act, &QAction::triggered, this, [name, isOpen]()
                {
                    if (!isOpen) openPanel(name);
                    else if (auto* c = containerHolding(name)) { c->raise(); c->activateWindow(); }
                });
            }
        });

        outerLayout->addWidget(m_menuBar);
    }

    m_body = new EditorBody(this);
    wireBody(m_body);

    if (isMainWindow)
    {
        // Build the pin-aware layout:
        //   HBox: [leftPinBar | workVSplitter | rightPinBar]
        //   workVSplitter (V): [topHSplitter]
        //   topHSplitter (H): [m_body]  (TL/TR panels inserted when pinned-open)
        m_leftPinBar  = new PinBar(true,  this);
        m_rightPinBar = new PinBar(false, this);

        m_topHSplitter = makeSplitter(Qt::Horizontal);
        m_topHSplitter->addWidget(m_body);

        m_bottomHSplitter = makeSplitter(Qt::Horizontal);
        m_bottomHSplitter->setParent(this);
        m_bottomHSplitter->hide();

        m_workVSplitter = makeSplitter(Qt::Vertical);
        m_workVSplitter->addWidget(m_topHSplitter);

        auto* centerWidget = new QWidget(this);
        auto* centerLayout = new QHBoxLayout(centerWidget);
        centerLayout->setContentsMargins(0, 0, 0, 0);
        centerLayout->setSpacing(0);
        centerLayout->addWidget(m_leftPinBar);
        centerLayout->addWidget(m_workVSplitter, 1);
        centerLayout->addWidget(m_rightPinBar);
        centerWidget->setLayout(centerLayout);

        outerLayout->addWidget(centerWidget, 1);

        // Status bar — anchored at the bottom, never overlaps pin icons.
        auto* statusBar = new QWidget(this);
        statusBar->setObjectName("StatusBar");
        statusBar->setFixedHeight(22);
        statusBar->setStyleSheet(
            "QWidget#StatusBar { background-color: #007acc; }"
            "QLabel { color: #ffffff; font-size: 10px; padding: 0 4px; }");

        auto* sbLayout = new QHBoxLayout(statusBar);
        sbLayout->setContentsMargins(4, 0, 0, 0);
        sbLayout->setSpacing(0);

        auto* versionLabel = new QLabel(
            QStringLiteral("O3DE 25.10.2  │  UX Mockup"), statusBar);
        sbLayout->addWidget(versionLabel);
        sbLayout->addStretch(1);

        m_statusLabel = new QLabel(QStringLiteral("Ready"), statusBar);
        sbLayout->addWidget(m_statusLabel);

        auto* grip = new QSizeGrip(statusBar);
        grip->setFixedSize(16, 22);
        grip->setStyleSheet("QSizeGrip { background: transparent; }");
        sbLayout->addWidget(grip);

        statusBar->setLayout(sbLayout);
        outerLayout->addWidget(statusBar);

        qApp->installEventFilter(this);
    }
    else
    {
        outerLayout->addWidget(m_body, 1);
    }

    setLayout(outerLayout);

    registry().append(this);
}

EditorWindowContainer::~EditorWindowContainer()
{
    if (m_isMainWindow)
        qApp->removeEventFilter(this);
    registry().removeAll(this);
    if (s_mostRecent == this) s_mostRecent = nullptr;
    if (s_mainWindow == this) s_mainWindow = nullptr;
}

bool EditorWindowContainer::isMainWindow() const { return m_isMainWindow; }

EditorBody* EditorWindowContainer::body() const { return m_body; }

bool EditorWindowContainer::hasRoom() const
{
    return m_body->panels().size() < 50;
}

void EditorWindowContainer::addPanel(EditorWindow* panel)    { m_body->addPanel(panel); }
void EditorWindowContainer::removePanel(EditorWindow* panel) { m_body->removePanel(panel); }
const QVector<EditorWindow*>& EditorWindowContainer::panels() const { return m_body->panels(); }

QVector<EditorBody*> EditorWindowContainer::allBodies() const
{
    const auto list = findChildren<EditorBody*>();
    return QVector<EditorBody*>(list.begin(), list.end());
}

// -------------------------------------------------------------------------
// Pin system
// -------------------------------------------------------------------------

QRect EditorWindowContainer::workAreaRect() const
{
    constexpr int k_statusBarH = 22;
    const int top    = m_menuBar ? m_menuBar->height() : 0;
    const int bottom = m_isMainWindow ? height() - k_statusBarH : height();
    const int left   = m_leftPinBar  ? m_leftPinBar->width()   : 0;
    const int right  = m_rightPinBar ? width() - m_rightPinBar->width() : width();
    return { left, top, right - left, bottom - top };
}

QRect EditorWindowContainer::overlayRect(PinPosition pos) const
{
    const QRect wa = workAreaRect();
    switch (pos)
    {
    case PinPosition::TopLeft:
        return { wa.left(), wa.top(),
                 k_pinOverlayWidth, wa.height() };
    case PinPosition::TopRight:
        return { wa.right() - k_pinOverlayWidth, wa.top(),
                 k_pinOverlayWidth, wa.height() };
    case PinPosition::BottomLeft:
    case PinPosition::BottomRight:
        return { wa.left(), wa.bottom() - k_pinOverlayHeight,
                 wa.width(), k_pinOverlayHeight };
    }
    return {};
}

void EditorWindowContainer::pinPanel(EditorWindow* panel, PinPosition pos)
{
    if (!m_isMainWindow)
    {
        if (s_mainWindow) s_mainWindow->pinPanel(panel, pos);
        return;
    }

    const int idx = static_cast<int>(pos);

    // Remove from any body that currently owns it.
    for (EditorBody* body : allBodies())
    {
        if (body->panels().contains(panel))
        {
            body->removePanel(panel);
            break;
        }
    }

    // Create PinPanel for this corner on first use.
    if (!m_pinPanels[idx])
    {
        m_pinPanels[idx] = new PinPanel(pos, this);
        m_pinPanels[idx]->hide();

        m_pinPanels[idx]->onPinToggled = [this, idx]()
        {
            if (m_pinPanels[idx]->isPinnedOpen())
                moveToPinnedOpen(idx);
            else
                showOverlay(idx);
        };

        m_pinPanels[idx]->onUnpin = [this, idx]()
        {
            EditorWindow* p = m_pinPanels[idx]->activePanel();
            if (p) QTimer::singleShot(0, this, [this, p]() { unpinPanel(p); });
        };
    }

    m_pinPanels[idx]->addPanel(panel);
    m_pinPanels[idx]->setActivePanel(panel);

    // Register icon in the pin bar.
    const bool topSection = (pos == PinPosition::TopLeft || pos == PinPosition::TopRight);
    const bool isLeft     = (pos == PinPosition::TopLeft || pos == PinPosition::BottomLeft);
    PinBar* bar = isLeft ? m_leftPinBar : m_rightPinBar;

    bar->addButton(panel, topSection,
        [this, pos](EditorWindow* p) { activatePanelAtCorner(pos, p); },
        [this](EditorWindow* p)
        {
            QTimer::singleShot(0, this, [this, p]() { unpinPanel(p); });
        });

    // Show as overlay initially.
    showOverlay(idx);
}

void EditorWindowContainer::unpinPanel(EditorWindow* panel)
{
    if (!m_isMainWindow)
    {
        if (s_mainWindow) s_mainWindow->unpinPanel(panel);
        return;
    }

    // Find which corner holds this panel.
    int cornerIdx = -1;
    for (int i = 0; i < 4; ++i)
    {
        if (m_pinPanels[i] && m_pinPanels[i]->panels().contains(panel))
        {
            cornerIdx = i;
            break;
        }
    }
    if (cornerIdx < 0) return;

    PinPanel* pp = m_pinPanels[cornerIdx];
    const PinPosition pos = pp->position();

    // Collapse if overlay/pinned-open.
    if (m_pinStates[cornerIdx] != PinState::Collapsed)
        moveToCollapsed(cornerIdx);

    // Remove from pin panel.
    pp->removePanel(panel);

    // Remove icon from pin bar.
    const bool isLeft = (pos == PinPosition::TopLeft || pos == PinPosition::BottomLeft);
    PinBar* bar = isLeft ? m_leftPinBar : m_rightPinBar;
    bar->removeButton(panel);

    // If pin panel is now empty, clean it up.
    if (pp->isEmpty())
    {
        delete m_pinPanels[cornerIdx];
        m_pinPanels[cornerIdx] = nullptr;
        m_pinStates[cornerIdx] = PinState::Collapsed;
    }

    // Return panel to dock.
    EditorWindowContainer* target = mostRecentWithRoom();
    if (!target) { target = new EditorWindowContainer(false); target->show(); }
    target->addPanel(panel);
}

void EditorWindowContainer::activatePanelAtCorner(PinPosition pos, EditorWindow* panel)
{
    const int idx = static_cast<int>(pos);
    if (!m_pinPanels[idx]) return;

    const bool samePanel = (m_pinPanels[idx]->activePanel() == panel);
    m_pinPanels[idx]->setActivePanel(panel);

    const bool isLeft = (pos == PinPosition::TopLeft || pos == PinPosition::BottomLeft);
    PinBar* bar = isLeft ? m_leftPinBar : m_rightPinBar;
    for (EditorWindow* p : m_pinPanels[idx]->panels())
        bar->setHighlighted(p, p == panel);

    switch (m_pinStates[idx])
    {
    case PinState::Collapsed:
        showOverlay(idx);
        break;
    case PinState::Overlay:
        if (samePanel)
            hideOverlay(idx);  // click same icon again → dismiss
        // different panel: overlay stays open, active panel already switched above
        break;
    case PinState::PinnedOpen:
        break;
    }
}

void EditorWindowContainer::showOverlay(int idx)
{
    PinPanel* pp = m_pinPanels[idx];
    if (!pp) return;

    // If pinned-open, toggling goes to collapsed instead of overlay.
    if (m_pinStates[idx] == PinState::PinnedOpen)
    {
        moveToCollapsed(idx);
        return;
    }
    if (m_pinStates[idx] == PinState::Overlay)
    {
        hideOverlay(idx);
        return;
    }

    // Bring to overlay mode: parent is this container, positioned at the edge.
    pp->setParent(this);
    pp->setPinnedOpen(false);
    pp->setOverlayMode(true);
    const QRect r = overlayRect(static_cast<PinPosition>(idx));
    pp->setGeometry(r);
    pp->show();
    pp->raise();
    m_pinStates[idx] = PinState::Overlay;

    // Highlight icon.
    const PinPosition pos = static_cast<PinPosition>(idx);
    const bool isLeft = (pos == PinPosition::TopLeft || pos == PinPosition::BottomLeft);
    PinBar* bar = isLeft ? m_leftPinBar : m_rightPinBar;
    if (pp->activePanel()) bar->setHighlighted(pp->activePanel(), true);
}

void EditorWindowContainer::hideOverlay(int idx)
{
    PinPanel* pp = m_pinPanels[idx];
    if (!pp || m_pinStates[idx] != PinState::Overlay) return;

    pp->setOverlayMode(false);
    pp->hide();
    m_pinStates[idx] = PinState::Collapsed;

    const PinPosition pos = static_cast<PinPosition>(idx);
    const bool isLeft = (pos == PinPosition::TopLeft || pos == PinPosition::BottomLeft);
    PinBar* bar = isLeft ? m_leftPinBar : m_rightPinBar;
    if (pp->activePanel()) bar->setHighlighted(pp->activePanel(), false);
}

void EditorWindowContainer::moveToPinnedOpen(int idx)
{
    PinPanel* pp = m_pinPanels[idx];
    if (!pp) return;

    // Remove from overlay parent (currently 'this').
    pp->hide();
    pp->setOverlayMode(false);
    pp->setPinnedOpen(true);

    const PinPosition pos = static_cast<PinPosition>(idx);
    switch (pos)
    {
    case PinPosition::TopLeft:
        m_topHSplitter->insertWidget(0, pp);
        break;
    case PinPosition::TopRight:
        m_topHSplitter->addWidget(pp);
        break;
    case PinPosition::BottomLeft:
        ensureBottomSplitter();
        m_bottomHSplitter->insertWidget(0, pp);
        break;
    case PinPosition::BottomRight:
        ensureBottomSplitter();
        m_bottomHSplitter->addWidget(pp);
        break;
    }

    pp->show();
    m_pinStates[idx] = PinState::PinnedOpen;

    // Give the new panel a reasonable initial size via setSizes.
    QTimer::singleShot(0, this, [this, idx, pos]()
    {
        if (!m_pinPanels[idx]) return;
        const int target = (pos == PinPosition::BottomLeft || pos == PinPosition::BottomRight)
                           ? k_pinOverlayHeight : k_pinOverlayWidth;
        auto adjust = [&](QSplitter* sp, int newChildSize, int childIdx)
        {
            if (!sp || sp->count() < 2) return;
            QList<int> sizes = sp->sizes();
            if (childIdx >= 0 && childIdx < sizes.size())
            {
                const int total = sp->orientation() == Qt::Horizontal
                                  ? sp->width() : sp->height();
                const int other = total - target;
                sizes[childIdx] = target;
                sizes[childIdx == 0 ? 1 : 0] = other;
                sp->setSizes(sizes);
            }
        };
        if (pos == PinPosition::TopLeft)
            adjust(m_topHSplitter, target, 0);
        else if (pos == PinPosition::TopRight)
            adjust(m_topHSplitter, target, m_topHSplitter->count() - 1);
        else if (pos == PinPosition::BottomLeft)
            adjust(m_bottomHSplitter, target, 0);
        else
            adjust(m_bottomHSplitter, target, m_bottomHSplitter->count() - 1);

        // Give the bottom half of the V splitter a reasonable height.
        if ((pos == PinPosition::BottomLeft || pos == PinPosition::BottomRight)
            && m_workVSplitter->count() == 2)
        {
            const int sh = m_workVSplitter->height();
            m_workVSplitter->setSizes({ sh - k_pinOverlayHeight, k_pinOverlayHeight });
        }
    });

    const bool isLeft = (pos == PinPosition::TopLeft || pos == PinPosition::BottomLeft);
    PinBar* bar = isLeft ? m_leftPinBar : m_rightPinBar;
    if (pp->activePanel()) bar->setHighlighted(pp->activePanel(), true);
}

void EditorWindowContainer::moveToCollapsed(int idx)
{
    PinPanel* pp = m_pinPanels[idx];
    if (!pp) return;

    const PinState state = m_pinStates[idx];
    if (state == PinState::PinnedOpen)
    {
        // Remove from splitter by reparenting.
        pp->setParent(this);
        pp->setPinnedOpen(false);
        pp->hide();

        const PinPosition pos = static_cast<PinPosition>(idx);
        if (pos == PinPosition::BottomLeft || pos == PinPosition::BottomRight)
            maybeRemoveBottomSplitter();
    }
    else if (state == PinState::Overlay)
    {
        pp->hide();
    }

    m_pinStates[idx] = PinState::Collapsed;

    const PinPosition pos = static_cast<PinPosition>(idx);
    const bool isLeft = (pos == PinPosition::TopLeft || pos == PinPosition::BottomLeft);
    PinBar* bar = isLeft ? m_leftPinBar : m_rightPinBar;
    if (pp->activePanel()) bar->setHighlighted(pp->activePanel(), false);
}

void EditorWindowContainer::ensureBottomSplitter()
{
    for (int i = 0; i < m_workVSplitter->count(); ++i)
        if (m_workVSplitter->widget(i) == m_bottomHSplitter) return;
    m_workVSplitter->addWidget(m_bottomHSplitter);
    m_bottomHSplitter->show();
}

void EditorWindowContainer::maybeRemoveBottomSplitter()
{
    const bool blOpen = (m_pinStates[static_cast<int>(PinPosition::BottomLeft)]
                         == PinState::PinnedOpen);
    const bool brOpen = (m_pinStates[static_cast<int>(PinPosition::BottomRight)]
                         == PinState::PinnedOpen);
    if (!blOpen && !brOpen)
    {
        m_bottomHSplitter->setParent(this);
        m_bottomHSplitter->hide();
    }
}

bool EditorWindowContainer::eventFilter(QObject* watched, QEvent* event)
{
    if (!m_isMainWindow)
        return QWidget::eventFilter(watched, event);

    if (event->type() == QEvent::MouseButtonPress)
    {
        const QPoint gp = static_cast<QMouseEvent*>(event)->globalPos();

        // If the click lands on a PinIconButton, skip dismissal — the button's
        // onClick will handle toggling the overlay.
        QWidget* under = QApplication::widgetAt(gp);
        bool onPinButton = false;
        while (under)
        {
            if (dynamic_cast<PinIconButton*>(under)) { onPinButton = true; break; }
            under = under->parentWidget();
        }

        if (!onPinButton)
        {
            // Dismiss any open overlay when the user clicks outside it.
            for (int i = 0; i < 4; ++i)
            {
                if (m_pinStates[i] != PinState::Overlay || !m_pinPanels[i])
                    continue;
                PinPanel* pp = m_pinPanels[i];
                const QRect panelGlobal(pp->mapToGlobal(QPoint(0, 0)), pp->size());
                if (!panelGlobal.contains(gp))
                    hideOverlay(i);
            }
        }
    }

    (void)watched;
    return QWidget::eventFilter(watched, event);
}

// -------------------------------------------------------------------------
// Splitting
// -------------------------------------------------------------------------

EditorBody* EditorWindowContainer::splitBody(EditorBody* target, Qt::Edge edge)
{
    const Qt::Orientation orient =
        (edge == Qt::LeftEdge || edge == Qt::RightEdge) ? Qt::Horizontal : Qt::Vertical;

    // Capture target dimensions before reparenting — the splitter won't have a
    // valid size yet when setSizes is called synchronously.
    const int targetDim = orient == Qt::Horizontal ? target->width() : target->height();

    auto* newBody = new EditorBody();
    newBody->setMinimumSize(50, 50);
    wireBody(newBody);

    auto* splitter = makeSplitter(orient);

    QWidget* targetParent = target->parentWidget();
    if (auto* parentSplitter = qobject_cast<QSplitter*>(targetParent))
    {
        const int idx = parentSplitter->indexOf(target);
        parentSplitter->insertWidget(idx, splitter);
    }
    else
    {
        layout()->replaceWidget(target, splitter);
    }

    if (edge == Qt::LeftEdge || edge == Qt::TopEdge)
    {
        splitter->addWidget(newBody);
        splitter->addWidget(target);
    }
    else
    {
        splitter->addWidget(target);
        splitter->addWidget(newBody);
    }

    splitter->show();

    // Defer setSizes until geometry is settled, using the pre-reparent dimension.
    QTimer::singleShot(0, splitter, [splitter, targetDim]()
    {
        const int half = qMax(50, targetDim / 2);
        splitter->setSizes({ half, targetDim - half });
    });

    return newBody;
}

// -------------------------------------------------------------------------
// Static registry helpers
// -------------------------------------------------------------------------

EditorWindowContainer* EditorWindowContainer::containerForBody(EditorBody* body)
{
    QWidget* w = body ? body->parentWidget() : nullptr;
    while (w)
    {
        if (auto* c = qobject_cast<EditorWindowContainer*>(w)) return c;
        w = w->parentWidget();
    }
    return nullptr;
}

void EditorWindowContainer::openPanel(const QString& name)
{
    if (!EditorWindowRegistry::isRegistered(name))
    {
        AZ_Warning("EditorWindowContainer", false, "openPanel: '%s' is not registered.",
            name.toUtf8().constData());
        return;
    }

    const PanelDescriptor* desc = nullptr;
    for (const PanelDescriptor& d : EditorWindowRegistry::getAllPanels())
        if (d.name == name) { desc = &d; break; }
    if (!desc) return;

    for (const QString& req : desc->requiredPanels)
        if (!containerHolding(req)) openPanel(req);

    EditorWindowContainer* target = nullptr;
    switch (desc->openIn)
    {
    case PanelDescriptor::OpenIn::NewWindow:
        target = new EditorWindowContainer(false);
        target->show();
        break;
    case PanelDescriptor::OpenIn::NextTo:
        if (!desc->openNextTo.isEmpty()) target = containerHolding(desc->openNextTo);
        if (!target || !target->hasRoom()) target = mostRecentWithRoom();
        break;
    default:
        target = mostRecentWithRoom();
        break;
    }
    if (!target || !target->hasRoom())
    {
        target = new EditorWindowContainer(false);
        target->show();
    }

    EditorWindow* panel = EditorWindowRegistry::createPanel(name);
    if (panel) target->addPanel(panel);
}

EditorWindowContainer* EditorWindowContainer::mostRecentWithRoom()
{
    if (s_mostRecent && s_mostRecent->hasRoom()) return s_mostRecent;
    for (EditorWindowContainer* c : registry())
        if (c->hasRoom()) return c;
    return nullptr;
}

EditorWindowContainer* EditorWindowContainer::containerHolding(const QString& panelName)
{
    for (EditorWindowContainer* c : registry())
    {
        for (const EditorWindow* p : c->panels())
            if (p->panelName() == panelName) return c;

        // Also check pinned panels.
        for (int i = 0; i < 4; ++i)
            if (c->m_pinPanels[i])
                for (const EditorWindow* p : c->m_pinPanels[i]->panels())
                    if (p->panelName() == panelName) return c;
    }
    return nullptr;
}

const QVector<EditorWindowContainer*>& EditorWindowContainer::allContainers()
{
    return registry();
}

EditorWindowContainer* EditorWindowContainer::mainWindow() { return s_mainWindow; }

EditorWindowContainer* EditorWindowContainer::createMainWindow()
{
    if (s_mainWindow) return s_mainWindow;
    s_mainWindow = new EditorWindowContainer(true);
    s_mostRecent = s_mainWindow;
    return s_mainWindow;
}

// -------------------------------------------------------------------------
// Qt events
// -------------------------------------------------------------------------

void EditorWindowContainer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    // Reposition any open overlays.
    for (int i = 0; i < 4; ++i)
    {
        if (m_pinStates[i] == PinState::Overlay && m_pinPanels[i])
            m_pinPanels[i]->setGeometry(overlayRect(static_cast<PinPosition>(i)));
    }
}

void EditorWindowContainer::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange && isActiveWindow())
        s_mostRecent = this;
}

void EditorWindowContainer::closeEvent(QCloseEvent* event)
{
    if (!m_isMainWindow) relocateNoClosePanels();
    QWidget::closeEvent(event);
}

// -------------------------------------------------------------------------
// Private helpers
// -------------------------------------------------------------------------

void EditorWindowContainer::relocateNoClosePanels()
{
    QVector<EditorWindow*> toRelocate;
    for (EditorWindow* p : m_body->panels())
        if (p->isNoClose()) toRelocate.append(p);
    for (EditorWindow* p : toRelocate)
    {
        m_body->removePanel(p);
        EditorWindowContainer* target = mostRecentWithRoom();
        if (!target) { target = new EditorWindowContainer(false); target->show(); }
        target->addPanel(p);
    }
}

void EditorWindowContainer::wireBody(EditorBody* body)
{
    connect(body, &EditorBody::becameEmpty, this, [this, body]()
    {
        QTimer::singleShot(0, this, [this, body]() { handleBodyEmpty(body); });
    });

    connect(body, &EditorBody::panelPinRequested, this,
            [this](EditorWindow* panel, PinPosition pos)
    {
        if (m_isMainWindow)
            pinPanel(panel, pos);
        else if (s_mainWindow)
            s_mainWindow->pinPanel(panel, pos);
    });
}

void EditorWindowContainer::handleBodyEmpty(EditorBody* body)
{
    if (!body) return;
    QWidget* parent = body->parentWidget();
    if (auto* splitter = qobject_cast<QSplitter*>(parent))
    {
        body->hide();
        body->setParent(nullptr);
        body->deleteLater();

        if (splitter->count() == 1)
        {
            QWidget* surviving = splitter->widget(0);
            QWidget* splitterParent = splitter->parentWidget();
            if (auto* grandSplitter = qobject_cast<QSplitter*>(splitterParent))
            {
                const int idx = grandSplitter->indexOf(splitter);
                grandSplitter->insertWidget(idx, surviving);
            }
            else
            {
                layout()->replaceWidget(splitter, surviving);
            }
            splitter->setParent(nullptr);
            splitter->deleteLater();
        }
    }
    else
    {
        if (!m_isMainWindow) deleteLater();
    }
}

// -------------------------------------------------------------------------
// Layout serialization
// -------------------------------------------------------------------------

void EditorWindowContainer::saveLayoutToFile()
{
    const QString path = QFileDialog::getSaveFileName(
        nullptr, tr("Save Layout"), QString(), tr("JSON Layout (*.json)"));
    if (path.isEmpty()) return;

    QJsonArray windows;
    for (const EditorWindowContainer* c : allContainers())
    {
        QJsonObject win;
        win[QStringLiteral("isMainWindow")] = c->m_isMainWindow;
        const QRect geo = c->geometry();
        win[QStringLiteral("x")] = geo.x(); win[QStringLiteral("y")] = geo.y();
        win[QStringLiteral("w")] = geo.width(); win[QStringLiteral("h")] = geo.height();

        // Find the dock root widget. For main windows, skip PinPanel children of
        // the top H splitter (pin state is not persisted across sessions).
        QWidget* dockRoot = c->m_body;
        if (c->m_topHSplitter)
        {
            for (int i = 0; i < c->m_topHSplitter->count(); ++i)
            {
                QWidget* w = c->m_topHSplitter->widget(i);
                if (!qobject_cast<PinPanel*>(w)) { dockRoot = w; break; }
            }
        }
        if (dockRoot)
            win[QStringLiteral("root")] = serializeBodyTree(dockRoot);

        windows.append(win);
    }

    QJsonObject doc;
    doc[QStringLiteral("version")] = 2;
    doc[QStringLiteral("windows")] = windows;

    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
        file.write(QJsonDocument(doc).toJson());
}

void EditorWindowContainer::loadLayoutFromFile()
{
    const QString path = QFileDialog::getOpenFileName(
        nullptr, tr("Load Layout"), QString(), tr("JSON Layout (*.json)"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonParseError err;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) return;

    const QJsonObject root = jsonDoc.object();
    if (root[QStringLiteral("version")].toInt() != 2) return;

    QVector<EditorWindowContainer*> toClose;
    for (EditorWindowContainer* c : registry())
        if (!c->m_isMainWindow) toClose.append(c);
    for (EditorWindowContainer* c : toClose) c->deleteLater();

    for (const QJsonValue v : root[QStringLiteral("windows")].toArray())
    {
        const QJsonObject winObj = v.toObject();
        const bool isMain = winObj[QStringLiteral("isMainWindow")].toBool();
        if (isMain && s_mainWindow)
            s_mainWindow->applyLayoutJson(winObj);
        else if (!isMain)
        {
            auto* c = new EditorWindowContainer(false);
            c->applyLayoutJson(winObj);
            c->show();
        }
    }
}

void EditorWindowContainer::applyLayoutJson(const QJsonObject& windowObj)
{
    QWidget* newRoot = deserializeBodyTree(windowObj[QStringLiteral("root")].toObject());

    // Wire newly created bodies.
    QVector<EditorBody*> bodies;
    if (newRoot) collectBodies(newRoot, bodies);
    for (EditorBody* b : bodies) wireBody(b);
    m_body = bodies.isEmpty() ? nullptr : bodies.first();

    if (m_isMainWindow && m_topHSplitter)
    {
        // Replace the body in the top H splitter.
        // Simple approach: clear and re-add.
        while (m_topHSplitter->count() > 0)
        {
            QWidget* w = m_topHSplitter->widget(0);
            w->setParent(nullptr);
            if (qobject_cast<EditorBody*>(w)) w->deleteLater();
        }
        if (newRoot) m_topHSplitter->addWidget(newRoot);
    }
    else if (!m_isMainWindow)
    {
        // Swap root in the simple non-main layout.
        auto* vl = qobject_cast<QVBoxLayout*>(layout());
        QWidget* oldRoot = nullptr;
        for (int i = 0; i < vl->count(); ++i)
        {
            QWidget* w = vl->itemAt(i)->widget();
            if (w && w != m_menuBar) { oldRoot = w; break; }
        }
        if (oldRoot) { vl->removeWidget(oldRoot); oldRoot->deleteLater(); }
        if (newRoot) vl->addWidget(newRoot, 1);
    }

    setGeometry(
        windowObj[QStringLiteral("x")].toInt(x()),
        windowObj[QStringLiteral("y")].toInt(y()),
        windowObj[QStringLiteral("w")].toInt(width()),
        windowObj[QStringLiteral("h")].toInt(height()));
}

} // namespace Heathen
