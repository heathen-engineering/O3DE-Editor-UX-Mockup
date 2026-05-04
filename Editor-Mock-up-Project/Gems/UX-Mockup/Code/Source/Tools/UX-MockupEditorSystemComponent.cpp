
#include <AzCore/Serialization/SerializeContext.h>

#include <AzToolsFramework/API/ViewPaneOptions.h>

#include "UX-MockupEditorSystemComponent.h"
#include "UX-MockupWidget.h"
#include "PanelTypes.h"

#include <UX-Mockup/UX-MockupTypeIds.h>
#include <UX-Mockup/EditorBody.h>
#include <UX-Mockup/EditorWindowContainer.h>
#include <UX-Mockup/EditorWindowRegistry.h>

#include <QSplitter>
#include <QTimer>

namespace UX_Mockup
{
    AZ_COMPONENT_IMPL(UX_MockupEditorSystemComponent, "UX_MockupEditorSystemComponent",
        UX_MockupEditorSystemComponentTypeId, BaseSystemComponent);

    void UX_MockupEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UX_MockupEditorSystemComponent, UX_MockupSystemComponent>()
                ->Version(0);
        }
    }

    UX_MockupEditorSystemComponent::UX_MockupEditorSystemComponent() = default;

    UX_MockupEditorSystemComponent::~UX_MockupEditorSystemComponent() = default;

    void UX_MockupEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("UX_MockupEditorService"));
    }

    void UX_MockupEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("UX_MockupEditorService"));
    }

    void UX_MockupEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void UX_MockupEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void UX_MockupEditorSystemComponent::Activate()
    {
        UX_MockupSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();

        using EWR = Heathen::EditorWindowRegistry;
        using PD  = Heathen::PanelDescriptor;

        // Viewport — noClose and noPin: must stay dockable, cannot be pinned to a corner.
        EWR::registerPanel({
            .name    = QStringLiteral("Scene View"),
            .category= QStringLiteral("Viewport"),
            .factory = []() -> Heathen::EditorWindow* { return new Heathen::SceneViewPanel(); },
            .noClose = true,
            .noPin   = true,
        });

        // Entity hierarchy
        EWR::registerPanel({
            .name     = QStringLiteral("Entity Outliner"),
            .category = QStringLiteral("Entities"),
            .factory  = []() -> Heathen::EditorWindow* { return new Heathen::EntityOutliner(); },
        });
        EWR::registerPanel({
            .name       = QStringLiteral("Inspector"),
            .category   = QStringLiteral("Entities"),
            .factory    = []() -> Heathen::EditorWindow* { return new Heathen::InspectorPanel(); },
            .openIn     = PD::OpenIn::NextTo,
            .openNextTo = QStringLiteral("Entity Outliner"),
        });

        // Assets and tools
        EWR::registerPanel({
            .name     = QStringLiteral("Asset Browser"),
            .category = QStringLiteral("Assets"),
            .factory  = []() -> Heathen::EditorWindow* { return new Heathen::AssetBrowserPanel(); },
        });
        EWR::registerPanel({
            .name     = QStringLiteral("Console"),
            .category = QStringLiteral("Tools"),
            .factory  = []() -> Heathen::EditorWindow* { return new Heathen::ConsolePanel(); },
        });

        // Demo: two script panels that prefer to live side-by-side.
        // Cool Script 2 must be registered first (Cool Script 1 depends on it).
        EWR::registerPanel({
            .name     = QStringLiteral("Cool Script 2"),
            .category = QStringLiteral("Scripts"),
            .factory  = []() -> Heathen::EditorWindow* { return new Heathen::AnotherCoolScript(); },
        });
        EWR::registerPanel({
            .name           = QStringLiteral("Cool Script 1"),
            .category       = QStringLiteral("Scripts"),
            .factory        = []() -> Heathen::EditorWindow* { return new Heathen::SomeCoolScript(); },
            .openIn         = PD::OpenIn::NextTo,
            .openNextTo     = QStringLiteral("Cool Script 2"),
            .requiredPanels = { QStringLiteral("Cool Script 2") },
        });

        // UI panels — demonstrating the variety of panel types
        EWR::registerPanel({
            .name     = QStringLiteral("Main Menu UI"),
            .category = QStringLiteral("UI"),
            .factory  = []() -> Heathen::EditorWindow* { return new Heathen::MainMenuUI(); },
        });
        EWR::registerPanel({
            .name     = QStringLiteral("Friends List UI"),
            .category = QStringLiteral("UI"),
            .factory  = []() -> Heathen::EditorWindow* { return new Heathen::FriendsListUI(); },
        });
        EWR::registerPanel({
            .name     = QStringLiteral("Leaderboard UI"),
            .category = QStringLiteral("UI"),
            .factory  = []() -> Heathen::EditorWindow* { return new Heathen::LeaderboardUI(); },
            .openIn   = PD::OpenIn::NewWindow,
        });
        EWR::registerPanel({
            .name     = QStringLiteral("Credits UI"),
            .category = QStringLiteral("UI"),
            .factory  = []() -> Heathen::EditorWindow* { return new Heathen::CreditsUI(); },
        });
    }

    void UX_MockupEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        UX_MockupSystemComponent::Deactivate();
    }

    void UX_MockupEditorSystemComponent::NotifyRegisterViews()
    {
        // Register a toolbar button so the workspace can always be opened/raised
        // from the O3DE editor. UX_MockupWidget::showEvent delegates to our
        // standalone EditorWindowContainer rather than acting as a dock panel.
        AzToolsFramework::ViewPaneOptions options;
        options.showOnToolsToolbar = true;
        options.toolbarIcon = ":/UX-Mockup/toolbar_icon.svg";
        AzToolsFramework::RegisterViewPane<UX_MockupWidget>("UX-Mockup", "UX-Mockup", options);

        // Build the default O3DE-style layout:
        //  [Entity Outliner] | [Scene View / Console] | [Inspector]
        using EWC = Heathen::EditorWindowContainer;
        using EWR = Heathen::EditorWindowRegistry;

        EWC* mainWindow = EWC::createMainWindow();
        mainWindow->show();
        mainWindow->raise();
        mainWindow->activateWindow();

        // The main body starts as the only body. We'll use it for Scene View.
        // Build the O3DE layout:
        //   Inspector (right, full height) | Scene View + Outliner (center) | bottom bar
        // Split order matters: Right first so Inspector gets the full height column.
        Heathen::EditorBody* centerBody = mainWindow->body();

        Heathen::EditorBody* rightBody  = mainWindow->splitBody(centerBody, Qt::RightEdge);
        Heathen::EditorBody* bottomBody = mainWindow->splitBody(centerBody, Qt::BottomEdge);
        Heathen::EditorBody* leftBody   = mainWindow->splitBody(centerBody, Qt::LeftEdge);

        // Populate each body.
        if (auto* p = EWR::createPanel(QStringLiteral("Entity Outliner")))
            leftBody->addPanel(p);
        if (auto* p = EWR::createPanel(QStringLiteral("Scene View")))
            centerBody->addPanel(p);
        if (auto* p = EWR::createPanel(QStringLiteral("Inspector")))
            rightBody->addPanel(p);
        if (auto* p = EWR::createPanel(QStringLiteral("Console")))
            bottomBody->addPanel(p);
        if (auto* p = EWR::createPanel(QStringLiteral("Asset Browser")))
            bottomBody->addPanel(p);

        // Defer size tuning until the window has been laid out and has real geometry.
        // After Right→Bottom→Left splits the structure is:
        //   sp1(H): [sp2(V), rightBody]
        //   sp2(V): [sp3(H), bottomBody]
        //   sp3(H): [leftBody, centerBody]
        QTimer::singleShot(0, mainWindow, [mainWindow]()
        {
            auto set2 = [](QSplitter* sp, int a, int b)
            {
                if (sp && sp->count() == 2) sp->setSizes({ a, b });
            };

            const auto splitters = mainWindow->findChildren<QSplitter*>();
            for (QSplitter* sp : splitters)
            {
                const int sw = sp->width();
                const int sh = sp->height();

                if (sp->orientation() == Qt::Horizontal && sp->count() == 2)
                {
                    // sp1: right child is rightBody (Inspector) → give it 350 px
                    if (qobject_cast<Heathen::EditorBody*>(sp->widget(1)))
                        set2(sp, sw - 350, 350);
                    // sp3: left child is leftBody (Outliner) → give it 350 px
                    else if (qobject_cast<Heathen::EditorBody*>(sp->widget(0)))
                        set2(sp, 350, sw - 350);
                }
                else if (sp->orientation() == Qt::Vertical && sp->count() == 2)
                {
                    // sp2: bottom child is bottomBody → give it 180 px
                    const int bottom = qMax(160, qMin(200, sh / 4));
                    set2(sp, sh - bottom, bottom);
                }
            }
        });
    }

} // namespace UX_Mockup
