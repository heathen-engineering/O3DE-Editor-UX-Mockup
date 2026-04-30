# O3DE Editor UX Mockup

A design-proving prototype for an ideal O3DE editor workspace. This project demonstrates a cleaner panel and window management system for O3DE, addressing key issues with the current dock system — especially on Wayland.

**This is not a full editor replacement.** It's a prototype Gem that proves a better pattern exists, for discussion with the O3DE community.

## The Problem

O3DE's current dock system has fundamental issues:

- Custom dock widgets don't work well (or at all) on Wayland — they're not real OS windows, they're custom decorations
- Panel management is clunky and inconsistent across platforms
- Layouts don't persist reliably
- Custom tools are hard to dock into the existing system
- Every dock window has its own menu, which is wrong

## The Vision

An editor workspace that works like a modern IDE:

- **Every window is a real OS window** — no custom window decorations, no dock widgets. Real OS windows on Wayland, Windows, X11 — everywhere it works.
- **Browser-like tab management** — panels are tabs within a container. Drag a tab to tear off into a new window.
- **Split bodies** — drag the border of a body to split it into two resizable regions (like VS/Rider/Unreal).
- **Auto-hiding tab bars** — the tab bar only appears when there's something to choose between. With one panel visible, the tab bar disappears and the panel fills the space. A small grab icon appears in the upper-left corner when you hover there, letting you summon the tab bar to grab the panel.
- **Edge-anchored panels** — panels can be anchored (collapsed) to any edge of their container, showing a small tab indicator on that edge. Click the edge tab to expand the panel over the content. Click off to collapse it back. Multiple anchored panels on the same edge each expand independently. This is exactly how the terminal works in VS, Rider, and CLion.
- **Menu only on the main window** — torn-off windows have no menu. The menu bar belongs to the main editor window.
- **Layout presets** — save and restore workspace configurations (which panels are open, where they are, which are tabbed/anchored, split ratios) as JSON.

## Getting Involved

This is primarily for discussion with the O3DE community. [Share your thoughts in the O3DE Discord](https://discord.com/channels/805939474655346758/1493595628809359480).

## Architecture

### EditorWindow (base class)
Every panel derives from this — Scene View, Inspector, Asset Browser, custom script panels, UI panels. Each is a registered dockable window.

```cpp
class SomePanel : public Heathen::EditorWindow
{
    void BuildContent() override { /* ... */ }
};
EditorWindowRegistry.register("Some Panel", SomePanel::create);
```

### EditorWindowContainer (real OS window)
A real OS window that holds 0-n EditorWindow children in a tabbed body. This is the core unit:

- **Main window** = container with menu bar
- **Torn-off windows** = container without menu bar (same body model)
- **Split body** = container subdivided into resizable regions

### Window Layout

```
┌─────────────────────────Main Window──────────────────────┐
│ Menu: File  Settings  Tools ...                          │
├──────────┬───────────────────────────────────┬───────────┤
│          │                                   │           │
│  [Body   │           Body                    │ [Body     │
│   A:     │           B                       │   C:      │
│  ┌─────┐ │  ┌───────────────────────┐        │  ┌─────┐  │
│  │Tab 1│ │  │  [Anchored:Top]       │        │  │Tab 1│  │
│  │Tab 2│ │  │  (expanded panel)     │        │  │Tab 2│  │
│  └─────┘ │  └───────────────────────┘        │  └─────┘  │
│          │                                   │           │
├──────────┤          2 tabs                   ├───────────┤
│          │       [Panel A] [Panel B]         │           │
│ [Left    │                                   │ [Anched   │
│  Edge    │                                   │  Edge     │
│  tab:    │                                   │  tab:     │
│  Panel]  │                                   │ Panel]    │
└──────────┴───────────────────────────────────┴───────────┘
```

## Prototype Status

### Design ✅ Complete
Full design spec written and documented in `memory/projects/ideal-editor-window.md`.

### Prototype Gem 🚧 In Progress
A working prototype is being built as an O3DE Gem:
- **Gem location:** `../Heathen-IdealEditorWindow/` (separate repository)
- **Gem scaffold:** `gem.json`, `CMakeLists.txt` ✅
- **EditorWindow (base class):** ✅
- **EditorWindowContainer (main window):** ✅
- **Tab bar, split, tear-off, anchor:** 🚧
- **Empty panel placeholders:** planned
- **Layout presets (JSON):** planned

### Panel Types (prototype)
All panels are empty styled placeholders — the value is in demonstrating the docking system, not content:
- Scene View (gray grid overlay)
- Entity Outliner (tree view)
- Inspector (property list)
- Asset Browser (column headers)
- Console (dark background, monospace)
- SomeCoolScript / AnotherCoolScript (mock script panels)
- MainMenuUI / FriendsListUI / LeaderboardUI (mock UI panels)

## Key Differences from O3DE Current

| Aspect | Current O3DE | This Mockup |
|--------|-------------|-------------|
| Windows | Custom dock widgets | Real OS windows |
| Docking | `AZ::Wt::DockWidget` | Custom tab body system |
| Wayland | Broken | Works |
| Tab management | Panel-specific | Universal |
| Menu | In each dock window | Main window only |
| Window hierarchy | Flat | Tree (body → tabs → splits) |
| Layout presets | No | Yes (JSON) |
| Collapse/anchor | Limited | Full (every panel) |
| Tear-off | No | Yes |
