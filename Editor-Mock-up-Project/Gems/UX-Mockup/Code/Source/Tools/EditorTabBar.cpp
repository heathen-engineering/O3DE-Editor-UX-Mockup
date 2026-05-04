#include <UX-Mockup/EditorTabBar.h>
#include <UX-Mockup/EditorBody.h>
#include <UX-Mockup/EditorWindow.h>
#include <UX-Mockup/PinSystem.h>
#include <UX-Mockup/TabDragController.h>

#include <QApplication>
#include <QContextMenuEvent>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyle>
#include <QTimer>

namespace {
    constexpr int k_tabHeight   = 28;
    constexpr int k_minTabWidth = 100;
    constexpr int k_maxTabWidth = 220;
    constexpr int k_textPadding = 52; // icon + label + close button + margins
    constexpr int k_iconSize    = 14;
}

namespace Heathen
{
    // -------------------------------------------------------------------------
    // EditorTab
    // -------------------------------------------------------------------------

    EditorTab::EditorTab(EditorWindow* panel, QWidget* parent)
        : QWidget(parent)
        , m_panel(panel)
    {
        setFixedHeight(k_tabHeight);
        setAutoFillBackground(true);
        setCursor(Qt::ArrowCursor);

        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(6, 0, 4, 0);
        layout->setSpacing(4);

        auto* iconLabel = new QLabel(this);
        iconLabel->setFixedSize(k_iconSize, k_iconSize);
        iconLabel->setPixmap(panel->tabIcon().pixmap(k_iconSize, k_iconSize));
        layout->addWidget(iconLabel);

        m_label = new QLabel(panel->panelName(), this);
        m_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(m_label);

        m_closeBtn = new QPushButton(QStringLiteral("×"), this);
        m_closeBtn->setFixedSize(16, 16);
        m_closeBtn->setFlat(true);
        m_closeBtn->setCursor(Qt::ArrowCursor);
        m_closeBtn->setToolTip(tr("Close panel"));
        m_closeBtn->hide();
        m_closeBtn->setStyleSheet(
            "QPushButton { border: none; background: transparent; color: #999; font-size: 12px; }"
            "QPushButton:hover { background: #c0392b; color: #fff; border-radius: 2px; }");

        // Suppress the close button for NoClose panels — the option should not exist.
        if (panel->isNoClose())
            m_closeBtn->setVisible(false);

        connect(m_closeBtn, &QPushButton::clicked, this, [this]()
        {
            if (onCloseRequested)
                onCloseRequested();
        });

        layout->addWidget(m_closeBtn);
        setLayout(layout);
        applyStyle();
    }

    EditorWindow* EditorTab::panel() const
    {
        return m_panel;
    }

    void EditorTab::setActive(bool active)
    {
        if (m_active == active)
            return;
        m_active = active;
        applyStyle();
    }

    bool EditorTab::isActive() const
    {
        return m_active;
    }

    void EditorTab::enterEvent(QEvent* /*event*/)
    {
        m_hovered = true;
        applyStyle();
    }

    void EditorTab::leaveEvent(QEvent* /*event*/)
    {
        m_hovered = false;
        applyStyle();
    }

    void EditorTab::mousePressEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton)
        {
            m_dragging     = false;
            m_dragStartPos = event->globalPos();
            if (onActivated)
                onActivated();
        }
        QWidget::mousePressEvent(event);
    }

    void EditorTab::mouseMoveEvent(QMouseEvent* event)
    {
        if (!m_dragging && (event->buttons() & Qt::LeftButton))
        {
            const int dist = (event->globalPos() - m_dragStartPos).manhattanLength();
            if (dist >= QApplication::startDragDistance())
            {
                m_dragging = true;
                if (onDragStarted)
                    onDragStarted(event->globalPos());
            }
        }
        QWidget::mouseMoveEvent(event);
    }

    void EditorTab::mouseReleaseEvent(QMouseEvent* event)
    {
        m_dragging = false;
        QWidget::mouseReleaseEvent(event);
    }

    void EditorTab::contextMenuEvent(QContextMenuEvent* event)
    {
        QMenu menu(this);

        if (!m_panel->isNoPin())
        {
            QMenu* moveTo = menu.addMenu(tr("Move To"));
            auto pin = [this](PinPosition pos)
            {
                return [this, pos]() { if (onPinRequested) onPinRequested(pos); };
            };
            moveTo->addAction(tr("Pin Top Left"),     pin(PinPosition::TopLeft));
            moveTo->addAction(tr("Pin Top Right"),    pin(PinPosition::TopRight));
            moveTo->addAction(tr("Pin Bottom Left"),  pin(PinPosition::BottomLeft));
            moveTo->addAction(tr("Pin Bottom Right"), pin(PinPosition::BottomRight));
        }

        if (!m_panel->isNoClose())
        {
            if (!menu.isEmpty())
                menu.addSeparator();
            menu.addAction(tr("Close"), [this]() { if (onCloseRequested) onCloseRequested(); });
        }

        if (!menu.isEmpty())
            menu.exec(event->globalPos());
    }

    void EditorTab::applyStyle()
    {
        QPalette pal = palette();

        if (m_active)
        {
            pal.setColor(QPalette::Window, QColor("#282828"));  // matches ContentArea bg
            m_label->setStyleSheet("color: #ffffff;");
        }
        else if (m_hovered)
        {
            pal.setColor(QPalette::Window, QColor("#2d2d2d"));
            m_label->setStyleSheet("color: #cccccc;");
        }
        else
        {
            pal.setColor(QPalette::Window, QColor("#242424"));
            m_label->setStyleSheet("color: #969696;");
        }

        // Close button visible only while hovered, never on active-but-not-hovered.
        if (!m_panel->isNoClose())
            m_closeBtn->setVisible(m_hovered);

        setPalette(pal);
    }

    // -------------------------------------------------------------------------
    // EditorTabBar
    // -------------------------------------------------------------------------

    EditorTabBar::EditorTabBar(QWidget* parent)
        : QWidget(parent)
    {
        setObjectName("EditorTabBar");
        setStyleSheet("QWidget#EditorTabBar { background-color: #1e1e1e; border-bottom: 1px solid #141414; }");
        setFixedHeight(0);  // starts hidden; shown via showAnimated()
    }

    void EditorTabBar::addTab(EditorWindow* panel)
    {
        auto* tab = new EditorTab(panel, this);

        tab->onActivated = [this, panel]()
        {
            setActiveTab(panel);
            emit tabActivated(panel);
        };

        tab->onCloseRequested = [this, panel]()
        {
            QTimer::singleShot(0, this, [this, panel]() { emit tabCloseRequested(panel); });
        };

        tab->onPinRequested = [this, panel](PinPosition pos)
        {
            QTimer::singleShot(0, this, [this, panel, pos]() { emit tabPinRequested(panel, pos); });
        };

        tab->onDragStarted = [this, tab](QPoint globalPos)
        {
            auto* body = qobject_cast<EditorBody*>(parent());
            TabDragController::instance()->startDrag(tab->panel(), body, globalPos);
        };

        m_tabs.append(tab);
        relayout();
    }

    void EditorTabBar::removeTab(EditorWindow* panel)
    {
        for (int i = 0; i < m_tabs.size(); ++i)
        {
            if (m_tabs[i]->panel() == panel)
            {
                delete m_tabs.takeAt(i);
                break;
            }
        }
        relayout();
    }

    void EditorTabBar::setActiveTab(EditorWindow* panel)
    {
        for (EditorTab* tab : m_tabs)
            tab->setActive(tab->panel() == panel);
    }

    void EditorTabBar::showAnimated(bool temporary)
    {
        stopAnimation();

        const int target = computeNaturalHeight();
        if (target == 0)
            return;

        show();

        m_animation = new QPropertyAnimation(this, "animatedHeight", this);
        m_animation->setStartValue(m_animatedHeight);
        m_animation->setEndValue(target);
        m_animation->setDuration(temporary ? 120 : 80);
        m_animation->setEasingCurve(temporary ? QEasingCurve::InOutCubic : QEasingCurve::OutQuad);
        connect(m_animation, &QPropertyAnimation::finished, this, [this]() { m_animation = nullptr; });
        m_animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void EditorTabBar::hideAnimated()
    {
        stopAnimation();

        m_animation = new QPropertyAnimation(this, "animatedHeight", this);
        m_animation->setStartValue(m_animatedHeight);
        m_animation->setEndValue(0);
        m_animation->setDuration(60);
        m_animation->setEasingCurve(QEasingCurve::OutQuad);
        connect(m_animation, &QPropertyAnimation::finished, this, [this]()
        {
            m_animation = nullptr;
            hide();
        });
        m_animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    QSize EditorTabBar::sizeHint() const
    {
        return { width(), m_animatedHeight };
    }

    QSize EditorTabBar::minimumSizeHint() const
    {
        return { 0, m_animatedHeight };
    }

    int EditorTabBar::animatedHeight() const
    {
        return m_animatedHeight;
    }

    void EditorTabBar::setAnimatedHeight(int h)
    {
        m_animatedHeight = h;
        setFixedHeight(h);
        updateGeometry();
    }

    void EditorTabBar::resizeEvent(QResizeEvent* event)
    {
        QWidget::resizeEvent(event);
        relayout();
    }

    void EditorTabBar::relayout()
    {
        if (m_tabs.isEmpty())
            return;

        const int w = qMax(1, width());
        int x = 0;
        int y = 0;

        for (EditorTab* tab : m_tabs)
        {
            const int tw = tabWidth(tab);
            if (x + tw > w && x > 0)
            {
                x = 0;
                y += k_tabHeight;
            }
            tab->setGeometry(x, y, tw, k_tabHeight);
            tab->show();
            x += tw;
        }

        // Update our own height to match the wrapped content, but only if we
        // are not mid-animation (the animation drives setAnimatedHeight itself).
        if (!m_animation && m_animatedHeight > 0)
        {
            const int natural = computeNaturalHeight();
            if (m_animatedHeight != natural)
                setAnimatedHeight(natural);
        }
    }

    int EditorTabBar::computeNaturalHeight() const
    {
        if (m_tabs.isEmpty())
            return 0;

        const int w = qMax(1, width());
        int x = 0;
        int rows = 1;

        for (const EditorTab* tab : m_tabs)
        {
            const int tw = tabWidth(tab);
            if (x + tw > w && x > 0)
            {
                ++rows;
                x = 0;
            }
            x += tw;
        }

        return rows * k_tabHeight;
    }

    int EditorTabBar::tabWidth(const EditorTab* tab) const
    {
        const QFontMetrics fm(tab->font());
        const int textW = fm.horizontalAdvance(tab->panel()->panelName());
        return qBound(k_minTabWidth, textW + k_textPadding, k_maxTabWidth);
    }

    void EditorTabBar::stopAnimation()
    {
        if (m_animation)
        {
            m_animation->stop();
            // DeleteWhenStopped will clean it up; null our pointer.
            m_animation = nullptr;
        }
    }

    int EditorTabBar::tabInsertionIndexAt(QPoint globalPos) const
    {
        for (int i = 0; i < m_tabs.size(); ++i)
        {
            const QPoint local = m_tabs[i]->mapFromGlobal(globalPos);
            if (m_tabs[i]->rect().contains(local))
                return (local.x() < m_tabs[i]->width() / 2) ? i : i + 1;
        }
        return m_tabs.size();
    }

    void EditorTabBar::reorderTab(EditorWindow* panel, int insertBefore)
    {
        int fromIdx = -1;
        for (int i = 0; i < m_tabs.size(); ++i)
            if (m_tabs[i]->panel() == panel) { fromIdx = i; break; }
        if (fromIdx < 0) return;

        EditorTab* tab = m_tabs.takeAt(fromIdx);
        if (insertBefore > fromIdx) --insertBefore;
        insertBefore = qBound(0, insertBefore, m_tabs.size());
        m_tabs.insert(insertBefore, tab);
        relayout();
    }

} // namespace Heathen
