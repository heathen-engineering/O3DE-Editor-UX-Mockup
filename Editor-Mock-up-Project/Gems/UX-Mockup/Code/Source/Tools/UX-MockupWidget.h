
#pragma once

#if !defined(Q_MOC_RUN)
#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <QWidget>
#endif

namespace UX_Mockup
{
    class UX_MockupWidget
        : public QWidget
    {
        Q_OBJECT
    public:
        explicit UX_MockupWidget(QWidget* parent = nullptr);
    };
} 
