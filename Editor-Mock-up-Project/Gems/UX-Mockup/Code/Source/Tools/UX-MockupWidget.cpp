
#include "UX-MockupWidget.h"

#include <UX-Mockup/EditorWindowContainer.h>

#include <QLabel>
#include <QShowEvent>
#include <QVBoxLayout>

namespace UX_Mockup
{
    UX_MockupWidget::UX_MockupWidget(QWidget* parent)
        : QWidget(parent)
    {
        auto* layout = new QVBoxLayout(this);
        auto* label  = new QLabel(tr("UX Mockup Workspace is open in its own window."), this);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
        setLayout(layout);
    }

    void UX_MockupWidget::showEvent(QShowEvent* event)
    {
        QWidget::showEvent(event);

        Heathen::EditorWindowContainer* win = Heathen::EditorWindowContainer::createMainWindow();
        win->show();
        win->raise();
        win->activateWindow();
    }
}

#include <moc_UX-MockupWidget.cpp>
