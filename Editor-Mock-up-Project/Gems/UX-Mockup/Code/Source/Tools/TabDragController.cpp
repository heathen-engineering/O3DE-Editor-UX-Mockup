#include <UX-Mockup/TabDragController.h>
#include <UX-Mockup/EditorBody.h>
#include <UX-Mockup/EditorWindow.h>
#include <UX-Mockup/EditorWindowContainer.h>

#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

namespace Heathen
{
    TabDragController* TabDragController::instance()
    {
        static TabDragController s_instance;
        return &s_instance;
    }

    bool TabDragController::isDragging() const
    {
        return m_active;
    }

    void TabDragController::startDrag(EditorWindow* panel, EditorBody* sourceBody, QPoint globalPos)
    {
        if (m_active || !panel || !sourceBody)
            return;

        m_panel      = panel;
        m_sourceBody = sourceBody;
        m_startPos   = globalPos;
        m_active     = true;

        // Frameless floating ghost that follows the cursor.
        auto* ghost = new QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        ghost->setAttribute(Qt::WA_TransparentForMouseEvents);
        ghost->setAttribute(Qt::WA_ShowWithoutActivating);
        ghost->setFixedSize(200, 28);
        ghost->setStyleSheet(
            "background-color: #3c3c3c; border: 1px solid #007acc; border-radius: 3px;");

        auto* label = new QLabel(panel->panelName(), ghost);
        label->setStyleSheet("color: #cccccc; font-size: 12px; padding: 0 8px;");
        label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        auto* layout = new QVBoxLayout(ghost);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(label);

        m_ghost = ghost;
        moveGhost(globalPos);
        ghost->show();

        qApp->installEventFilter(this);
    }

    bool TabDragController::eventFilter(QObject* /*obj*/, QEvent* event)
    {
        if (!m_active)
            return false;

        switch (event->type())
        {
        case QEvent::MouseMove:
            updateDrag(static_cast<QMouseEvent*>(event)->globalPos());
            return true;

        case QEvent::MouseButtonRelease:
            endDrag(static_cast<QMouseEvent*>(event)->globalPos());
            return true;

        case QEvent::KeyPress:
            if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)
            {
                cancelDrag();
                return true;
            }
            break;

        default:
            break;
        }
        return false;
    }

    void TabDragController::updateDrag(QPoint globalPos)
    {
        moveGhost(globalPos);
        applyVisuals(evaluateZone(globalPos), globalPos);
    }

    void TabDragController::endDrag(QPoint globalPos)
    {
        clearVisuals();
        if (m_ghost)
            delete m_ghost.data();

        qApp->removeEventFilter(this);
        m_active = false;

        const ZoneInfo zone = evaluateZone(globalPos);

        if (zone.type == ZoneType::Dock && zone.targetBody == m_sourceBody)
        {
            // Same body: reorder the tab based on drop position.
            const int insertIdx = m_sourceBody->tabInsertionIndexAt(globalPos);
            m_sourceBody->reorderPanel(m_panel, insertIdx);
        }
        else if (zone.type == ZoneType::Dock && zone.targetBody && zone.targetBody != m_sourceBody)
        {
            m_sourceBody->removePanel(m_panel);
            zone.targetBody->addPanel(m_panel);
        }
        else if (zone.type == ZoneType::Split && zone.targetBody)
        {
            auto* container = EditorWindowContainer::containerForBody(zone.targetBody);
            if (container)
            {
                EditorBody* newBody = container->splitBody(zone.targetBody, zone.splitEdge);
                m_sourceBody->removePanel(m_panel);
                newBody->addPanel(m_panel);
            }
            else
            {
                // Fallback (should not happen).
                auto* newContainer = new EditorWindowContainer(false, nullptr);
                m_sourceBody->removePanel(m_panel);
                newContainer->addPanel(m_panel);
                newContainer->move(globalPos - QPoint(100, 14));
                newContainer->resize(600, 400);
                newContainer->show();
            }
        }
        else if (zone.type == ZoneType::TearOff)
        {
            auto* container = new EditorWindowContainer(false, nullptr);
            m_sourceBody->removePanel(m_panel);
            container->addPanel(m_panel);
            container->move(globalPos - QPoint(100, 14));
            container->resize(600, 400);
            container->show();
        }
        m_panel      = nullptr;
        m_sourceBody = nullptr;
    }

    void TabDragController::cancelDrag()
    {
        clearVisuals();
        if (m_ghost)
            delete m_ghost.data();

        qApp->removeEventFilter(this);
        m_active     = false;
        m_panel      = nullptr;
        m_sourceBody = nullptr;
    }

    TabDragController::ZoneInfo TabDragController::evaluateZone(QPoint globalPos) const
    {
        // Priority: Dock (tab bar area) → Split (25% edges) → TearOff.
        for (auto* container : EditorWindowContainer::allContainers())
        {
            for (EditorBody* body : container->allBodies())
            {
                if (!body)
                    continue;

                const QRect bodyRect(body->mapToGlobal(QPoint(0, 0)), body->size());
                if (!bodyRect.contains(globalPos))
                    continue;

                // Dock zone: top 42 px strip (covers the tab bar regardless of visibility).
                if (QRect(bodyRect.left(), bodyRect.top(), bodyRect.width(), 42).contains(globalPos))
                    return { ZoneType::Dock, body, Qt::TopEdge };

                // Split zones: outer 25% of each edge, checked left → right → top → bottom.
                const int w25 = bodyRect.width()  / 4;
                const int h25 = bodyRect.height() / 4;

                if (QRect(bodyRect.left(),        bodyRect.top(), w25, bodyRect.height()).contains(globalPos))
                    return { ZoneType::Split, body, Qt::LeftEdge };
                if (QRect(bodyRect.right() - w25, bodyRect.top(), w25, bodyRect.height()).contains(globalPos))
                    return { ZoneType::Split, body, Qt::RightEdge };
                if (QRect(bodyRect.left(),        bodyRect.top(), bodyRect.width(), h25).contains(globalPos))
                    return { ZoneType::Split, body, Qt::TopEdge };
                if (QRect(bodyRect.left(), bodyRect.bottom() - h25, bodyRect.width(), h25).contains(globalPos))
                    return { ZoneType::Split, body, Qt::BottomEdge };

                // Center of the body: dock to it.
                return { ZoneType::Dock, body, Qt::TopEdge };
            }
        }

        return { ZoneType::TearOff, nullptr, Qt::TopEdge };
    }

    void TabDragController::applyVisuals(const ZoneInfo& zone, QPoint /*globalPos*/)
    {
        if (zone.type == ZoneType::TearOff || !zone.targetBody)
        {
            clearVisuals();
            return;
        }

        EditorBody* body = zone.targetBody;
        const QRect local = body->rect();
        QRect indicatorRect;

        if (zone.type == ZoneType::Dock)
        {
            indicatorRect = QRect(0, 0, local.width(), 42);
        }
        else
        {
            const int w25 = local.width()  / 4;
            const int h25 = local.height() / 4;
            switch (zone.splitEdge)
            {
            case Qt::LeftEdge:   indicatorRect = QRect(0,                  0, w25, local.height()); break;
            case Qt::RightEdge:  indicatorRect = QRect(local.width() - w25, 0, w25, local.height()); break;
            case Qt::TopEdge:    indicatorRect = QRect(0,                  0, local.width(), h25);  break;
            case Qt::BottomEdge: indicatorRect = QRect(0, local.height() - h25, local.width(), h25); break;
            default: break;
            }
        }

        // Reuse or create the indicator widget parented to the target body.
        if (!m_zoneIndicator || m_zoneIndicator->parent() != body)
        {
            delete m_zoneIndicator.data();
            m_zoneIndicator = new QWidget(body);
            m_zoneIndicator->setAttribute(Qt::WA_TransparentForMouseEvents);
            m_zoneIndicator->setStyleSheet(
                "background-color: rgba(0, 120, 212, 80);"
                "border: 2px solid #007acc;");
        }

        m_zoneIndicator->setGeometry(indicatorRect);
        m_zoneIndicator->show();
        m_zoneIndicator->raise();
    }

    void TabDragController::clearVisuals()
    {
        if (m_zoneIndicator)
            delete m_zoneIndicator.data();
    }

    void TabDragController::moveGhost(QPoint globalPos)
    {
        if (m_ghost)
            m_ghost->move(globalPos + QPoint(12, 4));
    }

} // namespace Heathen
