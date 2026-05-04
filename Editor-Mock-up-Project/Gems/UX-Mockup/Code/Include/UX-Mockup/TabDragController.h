#pragma once

#include <QObject>
#include <QPointer>
#include <QPoint>

namespace Heathen
{
    class EditorWindow;
    class EditorBody;

    // Singleton that owns all tab-drag state.
    // Installed as a QApplication event filter for the duration of each drag.
    // Zone priority (highest first): Dock → Split → TearOff.
    class TabDragController : public QObject
    {
        Q_OBJECT

    public:
        static TabDragController* instance();

        // Called by EditorTab when a drag gesture is detected.
        void startDrag(EditorWindow* panel, EditorBody* sourceBody, QPoint globalPos);
        bool isDragging() const;

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;

    private:
        TabDragController() = default;

        enum class ZoneType { TearOff, Split, Dock };

        struct ZoneInfo
        {
            ZoneType    type       = ZoneType::TearOff;
            EditorBody* targetBody = nullptr;
            Qt::Edge    splitEdge  = Qt::TopEdge;
        };

        void updateDrag(QPoint globalPos);
        void endDrag(QPoint globalPos);
        void cancelDrag();

        ZoneInfo evaluateZone(QPoint globalPos) const;
        void applyVisuals(const ZoneInfo& zone, QPoint globalPos);
        void clearVisuals();
        void moveGhost(QPoint globalPos);

        EditorWindow* m_panel      = nullptr;
        EditorBody*   m_sourceBody = nullptr;
        QPoint        m_startPos;
        bool          m_active = false;

        QPointer<QWidget> m_ghost;          // floating panel-name preview
        QPointer<QWidget> m_zoneIndicator;  // split/dock zone highlight overlay
    };

} // namespace Heathen
