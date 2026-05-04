#include <UX-Mockup/PinSystem.h>
#include <UX-Mockup/EditorWindow.h>

#include <QApplication>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyle>
#include <QVBoxLayout>

namespace {
constexpr int k_pinBarWidth  = 20;
constexpr int k_pinIconSize  = 20;
constexpr int k_titleBarH    = 22;
}

namespace Heathen
{

// -------------------------------------------------------------------------
// PinIconButton
// -------------------------------------------------------------------------

PinIconButton::PinIconButton(EditorWindow* panel, QWidget* parent)
    : QWidget(parent)
    , m_panel(panel)
{
    setFixedSize(k_pinIconSize, k_pinIconSize);
    setCursor(Qt::PointingHandCursor);
    setToolTip(panel->panelName());
}

EditorWindow* PinIconButton::panel() const { return m_panel; }

void PinIconButton::setHighlighted(bool on)
{
    if (m_highlighted == on)
        return;
    m_highlighted = on;
    update();
}

void PinIconButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && onClick)
        onClick();
    QWidget::mousePressEvent(event);
}

void PinIconButton::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    menu.addAction(tr("Move to Dockable"), this, [this]()
    {
        if (onUnpinRequested) onUnpinRequested();
    });
    menu.exec(event->globalPos());
}

void PinIconButton::enterEvent(QEvent*)
{
    m_hovered = true;
    update();
}

void PinIconButton::leaveEvent(QEvent*)
{
    m_hovered = false;
    update();
}

void PinIconButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    const QColor bg = m_highlighted ? QColor("#007acc")
                    : m_hovered     ? QColor("#3e3e3e")
                    :                 QColor("#252525");
    p.fillRect(rect(), bg);

    const QPixmap px = m_panel->tabIcon().pixmap(14, 14);
    p.drawPixmap((width() - px.width()) / 2, (height() - px.height()) / 2, px);
}

// -------------------------------------------------------------------------
// PinBar
// -------------------------------------------------------------------------

PinBar::PinBar(bool isLeft, QWidget* parent)
    : QWidget(parent)
{
    (void)isLeft;
    setFixedWidth(k_pinBarWidth);
    setStyleSheet("background-color: #1e1e1e;");

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 2, 0, 2);
    outer->setSpacing(0);

    auto* topW = new QWidget(this);
    m_topLayout = new QVBoxLayout(topW);
    m_topLayout->setContentsMargins(0, 0, 0, 0);
    m_topLayout->setSpacing(2);
    m_topLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    topW->setLayout(m_topLayout);

    auto* botW = new QWidget(this);
    m_botLayout = new QVBoxLayout(botW);
    m_botLayout->setContentsMargins(0, 0, 0, 0);
    m_botLayout->setSpacing(2);
    m_botLayout->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    botW->setLayout(m_botLayout);

    outer->addWidget(topW);
    outer->addStretch(1);
    outer->addWidget(botW);
    setLayout(outer);
}

void PinBar::addButton(EditorWindow* panel, bool topSection,
                       std::function<void(EditorWindow*)> onActivate,
                       std::function<void(EditorWindow*)> onUnpin)
{
    auto* btn = new PinIconButton(panel, this);
    btn->onClick          = [panel, onActivate]() { if (onActivate) onActivate(panel); };
    btn->onUnpinRequested = [panel, onUnpin]()    { if (onUnpin)    onUnpin(panel);    };
    m_buttons.append(btn);

    if (topSection)
        m_topLayout->addWidget(btn);
    else
        m_botLayout->insertWidget(0, btn);  // bottom-first ordering
}

void PinBar::removeButton(EditorWindow* panel)
{
    for (int i = 0; i < m_buttons.size(); ++i)
    {
        if (m_buttons[i]->panel() == panel)
        {
            delete m_buttons.takeAt(i);
            return;
        }
    }
}

void PinBar::setHighlighted(EditorWindow* panel, bool on)
{
    for (PinIconButton* btn : m_buttons)
        if (btn->panel() == panel)
            btn->setHighlighted(on);
}

// -------------------------------------------------------------------------
// PinPanel
// -------------------------------------------------------------------------

PinPanel::PinPanel(PinPosition pos, QWidget* parent)
    : QWidget(parent)
    , m_pos(pos)
{
    buildUI();
}

PinPanel::~PinPanel()
{
    // Reparent all content panels so they aren't destroyed with us.
    for (EditorWindow* p : m_panels)
    {
        p->hide();
        p->setParent(nullptr);
    }
}

void PinPanel::buildUI()
{
    setAutoFillBackground(true);
    setMouseTracking(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#252526"));
    setPalette(pal);

    // Title bar
    auto* titleBar = new QWidget(this);
    titleBar->setFixedHeight(k_titleBarH);
    titleBar->setStyleSheet("background-color: #1a1a1a;");

    auto* tl = new QHBoxLayout(titleBar);
    tl->setContentsMargins(6, 0, 4, 0);
    tl->setSpacing(4);

    m_titleLabel = new QLabel(QStringLiteral("—"), titleBar);
    m_titleLabel->setStyleSheet("color: #cccccc; font-size: 11px;");
    tl->addWidget(m_titleLabel, 1);

    m_pinBtn = new QPushButton(QStringLiteral("⊕"), titleBar);
    m_pinBtn->setFixedSize(16, 16);
    m_pinBtn->setFlat(true);
    m_pinBtn->setToolTip(tr("Keep open"));
    m_pinBtn->setStyleSheet(
        "QPushButton { border:none; background:transparent; color:#999; font-size:10px; }"
        "QPushButton:hover { background:#007acc; color:#fff; border-radius:2px; }");
    connect(m_pinBtn, &QPushButton::clicked, this, [this]()
    {
        setPinnedOpen(!m_pinnedOpen);
        if (onPinToggled) onPinToggled();
    });
    tl->addWidget(m_pinBtn);
    titleBar->setLayout(tl);

    // Content area — EditorWindow fills this via manual resize
    m_contentArea = new QWidget(this);
    m_contentArea->setObjectName("PinContentArea");
    m_contentArea->setAutoFillBackground(true);
    {
        QPalette cp = m_contentArea->palette();
        cp.setColor(QPalette::Window, QColor("#252526"));
        m_contentArea->setPalette(cp);
    }
    m_contentArea->installEventFilter(this);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(1, 1, 1, 1);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(titleBar);
    mainLayout->addWidget(m_contentArea, 1);
    setLayout(mainLayout);
}

void PinPanel::addPanel(EditorWindow* panel)
{
    if (!panel || m_panels.contains(panel))
        return;
    m_panels.append(panel);
    panel->setParent(m_contentArea);
    panel->hide();
    if (!m_activePanel)
        setActivePanel(panel);
}

void PinPanel::removePanel(EditorWindow* panel)
{
    if (!m_panels.contains(panel))
        return;
    const bool wasActive = (panel == m_activePanel);
    m_panels.removeAll(panel);
    panel->hide();
    panel->setParent(nullptr);
    if (wasActive)
    {
        m_activePanel = nullptr;
        if (!m_panels.isEmpty())
            setActivePanel(m_panels.last());
    }
    updateTitle();
}

void PinPanel::setActivePanel(EditorWindow* panel)
{
    if (!panel || !m_panels.contains(panel))
        return;
    if (m_activePanel)
        m_activePanel->hide();
    m_activePanel = panel;
    panel->setParent(m_contentArea);
    panel->show();
    positionActivePanel();
    updateTitle();
}

EditorWindow* PinPanel::activePanel() const { return m_activePanel; }

const QVector<EditorWindow*>& PinPanel::panels() const { return m_panels; }

bool PinPanel::isEmpty() const { return m_panels.isEmpty(); }

PinPosition PinPanel::position() const { return m_pos; }

bool PinPanel::isPinnedOpen() const { return m_pinnedOpen; }

void PinPanel::setPinnedOpen(bool open)
{
    m_pinnedOpen = open;
    m_pinBtn->setText(open ? QStringLiteral("⊗") : QStringLiteral("⊕"));
    m_pinBtn->setToolTip(open ? tr("Auto-hide") : tr("Keep open"));
}

void PinPanel::positionActivePanel()
{
    if (m_activePanel && m_contentArea)
        m_activePanel->setGeometry(m_contentArea->rect());
}

void PinPanel::updateTitle()
{
    m_titleLabel->setText(m_activePanel ? m_activePanel->panelName() : QStringLiteral("—"));
}

bool PinPanel::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_contentArea && event->type() == QEvent::Resize)
        positionActivePanel();
    return QWidget::eventFilter(watched, event);
}

void PinPanel::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    menu.addAction(m_pinnedOpen ? tr("Auto-hide") : tr("Keep open"), this, [this]()
    {
        setPinnedOpen(!m_pinnedOpen);
        if (onPinToggled) onPinToggled();
    });
    menu.addSeparator();
    menu.addAction(tr("Move to Dockable"), this, [this]()
    {
        if (onUnpin) onUnpin();
    });
    menu.exec(event->globalPos());
}

void PinPanel::setOverlayMode(bool overlay)
{
    if (m_overlayMode == overlay)
        return;
    m_overlayMode = overlay;
    update();
}

void PinPanel::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    const QColor borderCol = m_overlayMode ? QColor("#007acc") : QColor("#3c3c3c");
    p.setPen(QPen(borderCol, 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

bool PinPanel::inResizeZone(QPoint localPos) const
{
    if (!m_overlayMode)
        return false;
    constexpr int k_edge = 5;
    switch (m_pos)
    {
    case PinPosition::TopLeft:    return localPos.x() >= width() - k_edge;
    case PinPosition::TopRight:   return localPos.x() <= k_edge;
    case PinPosition::BottomLeft:
    case PinPosition::BottomRight: return localPos.y() <= k_edge;
    }
    return false;
}

Qt::CursorShape PinPanel::resizeCursor() const
{
    switch (m_pos)
    {
    case PinPosition::TopLeft:
    case PinPosition::TopRight:    return Qt::SizeHorCursor;
    case PinPosition::BottomLeft:
    case PinPosition::BottomRight: return Qt::SizeVerCursor;
    }
    return Qt::ArrowCursor;
}

void PinPanel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && inResizeZone(event->pos()))
    {
        m_resizing       = true;
        m_resizeAnchor   = event->globalPos();
        m_resizeStartRect = geometry();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void PinPanel::mouseMoveEvent(QMouseEvent* event)
{
    if (m_resizing)
    {
        const QPoint delta = event->globalPos() - m_resizeAnchor;
        QRect r = m_resizeStartRect;
        switch (m_pos)
        {
        case PinPosition::TopLeft:    r.setRight(r.right() + delta.x());  break;
        case PinPosition::TopRight:   r.setLeft(r.left()   + delta.x());  break;
        case PinPosition::BottomLeft:
        case PinPosition::BottomRight: r.setTop(r.top()    + delta.y());  break;
        }
        r.setWidth(qMax(r.width(), 80));
        r.setHeight(qMax(r.height(), 60));
        setGeometry(r);
        event->accept();
        return;
    }
    setCursor(inResizeZone(event->pos()) ? resizeCursor() : Qt::ArrowCursor);
    QWidget::mouseMoveEvent(event);
}

void PinPanel::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_resizing)
    {
        m_resizing = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

} // namespace Heathen
