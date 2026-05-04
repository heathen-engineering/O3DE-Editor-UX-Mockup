
#pragma once

#if !defined(Q_MOC_RUN)
#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <QWidget>
#endif

namespace UX_Mockup
{
    // Registered with O3DE's view pane system to provide a toolbar button.
    // When shown, it creates/raises the standalone EditorWindowContainer instead
    // of acting as a dock panel itself.
    class UX_MockupWidget
        : public QWidget
    {
        Q_OBJECT
    public:
        explicit UX_MockupWidget(QWidget* parent = nullptr);

    protected:
        void showEvent(QShowEvent* event) override;
    };
}
