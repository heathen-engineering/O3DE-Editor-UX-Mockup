#pragma once
#include <UX-Mockup/EditorWindow.h>

namespace Heathen
{
    // All panel types are empty styled placeholders — no real O3DE functionality.
    // The value is in demonstrating the docking system, not content fidelity.
    // Each class only overrides buildContent(); no new signals or slots are added,
    // so Q_OBJECT is intentionally omitted.

    class SceneViewPanel final : public EditorWindow
    {
    public:
        SceneViewPanel();
        void buildContent() override;
    };

    class EntityOutliner final : public EditorWindow
    {
    public:
        EntityOutliner();
        void buildContent() override;
    };

    class InspectorPanel final : public EditorWindow
    {
    public:
        InspectorPanel();
        void buildContent() override;
    };

    class AssetBrowserPanel final : public EditorWindow
    {
    public:
        AssetBrowserPanel();
        void buildContent() override;
        QIcon tabIcon() const override;
    };

    class ConsolePanel final : public EditorWindow
    {
    public:
        ConsolePanel();
        void buildContent() override;
    };

    class SomeCoolScript final : public EditorWindow
    {
    public:
        SomeCoolScript();
        void buildContent() override;
        QIcon tabIcon() const override;
    };

    class AnotherCoolScript final : public EditorWindow
    {
    public:
        AnotherCoolScript();
        void buildContent() override;
        QIcon tabIcon() const override;
    };

    class MainMenuUI final : public EditorWindow
    {
    public:
        MainMenuUI();
        void buildContent() override;
    };

    class FriendsListUI final : public EditorWindow
    {
    public:
        FriendsListUI();
        void buildContent() override;
    };

    class LeaderboardUI final : public EditorWindow
    {
    public:
        LeaderboardUI();
        void buildContent() override;
    };

    class CreditsUI final : public EditorWindow
    {
    public:
        CreditsUI();
        void buildContent() override;
    };

} // namespace Heathen
