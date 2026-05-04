# O3DE Editor UX Mockup

A design-proving prototype for an ideal O3DE editor workspace. This project demonstrates a cleaner panel and window management system for O3DE, addressing key issues with the current dock system — especially on Wayland.

**This is not a full editor replacement.** It is a prototype Gem that proves a better pattern exists, for discussion with the O3DE community.

---

## User Guide

### What you are looking at

When the Gem is activated in the O3DE editor, a standalone mock workspace window opens. This window contains placeholder panels — a fake Scene View, fake Entity Outliner, fake Inspector, and so on. None of the panels do real editor work; they exist to demonstrate the layout and interaction system.

Everything you can do with the window management (split, drag, pin, tear off) is the real implementation. The panel contents are not.

---

### Panels and tabs

Each body in the workspace holds one or more panels. When a body contains more than one panel, a tab bar appears along the top showing all panels in that body. Click a tab to bring that panel to the front.

**Closing a panel:** Hover over a tab to reveal the × button on the right side of that tab. Click it to close the panel. Panels marked as required (such as Scene View) cannot be closed and will not show a × button.

**Reordering tabs:** Click and drag a tab left or right within the same tab bar to reorder it.

---

### Auto-hiding tab bar and the grab handle

When a body contains only one panel, the tab bar hides itself — the panel fills the full body with no chrome overhead.

A small **grab handle button** appears in the upper-left corner of the body when you hover near there. Clicking it temporarily reveals the tab bar so you can grab the tab and drag the panel somewhere else. The tab bar auto-hides again when you click anywhere outside of it.

---

### Splitting and resizing

**Splitting:** Drag a tab out of its current body and drop it onto the edge zone of another body (the highlighted region that appears at the top, bottom, left, or right of a target body during a drag). This creates a new split region side-by-side with the target.

**Resizing splits:** Drag the splitter handle between any two split regions to resize them. The split ratio is preserved when the window is resized.

---

### Tearing off to a floating window

Drag a tab away from any body and drop it onto an empty area of the desktop (not onto another body's edge zone or tab bar). This tears the panel off into its own independent floating OS window. The floating window behaves identically to the main window — you can add more panels to it, split it, and resize it.

Close the floating window to destroy it. Any panels that were inside it and are marked no-close will be moved back to the main window automatically.

---

### The pin system

Panels can be **pinned to a corner** of the main window rather than living in the main dock area. This is useful for reference panels you want accessible but out of the way — equivalent to the terminal/output panels in VS Code, Rider, or CLion.

**Pinning a panel:** Right-click a tab and choose **Move To → Pin Top Left / Top Right / Bottom Left / Bottom Right**. The panel is removed from its body and a small icon appears in the pin bar on the left or right edge of the main window.

**Opening a pinned panel:** Click the icon in the pin bar. The panel slides out as an overlay on top of the workspace. The overlay expands to the full available width (for bottom corners) or full available height (for side corners). Click anywhere outside the overlay to collapse it again.

**Resizing the overlay:** While the panel is expanded as an overlay, drag the inner edge of the panel to resize it.

**Keeping a panel open (pinned-open mode):** While the panel is expanded, click the **⊕** button in the panel's title bar to keep it open permanently. In pinned-open mode the panel is inserted into the workspace splitter layout as a persistent column or row rather than floating as an overlay. Click **⊗** to return it to auto-hide overlay mode.

**Unpinning:** Right-click the panel's icon in the pin bar (or right-click inside the panel) and choose **Move to Dockable**. The panel returns to the main dock area.

---

### Panel icons

Each panel type defines its own icon, which appears in its tab and in the pin bar icon button. Developers implementing a new panel type override `EditorWindow::tabIcon()` to return any `QIcon`.

---

## The Problem

O3DE's current dock system has fundamental issues:

- Custom dock widgets do not work well (or at all) on Wayland — they are not real OS windows, they are custom decorations
- Panel management is clunky and inconsistent across platforms
- Layouts do not persist reliably
- Custom tools are hard to dock into the existing system
- Every dock window has its own menu, which is wrong

---

## The Vision

An editor workspace that works like a modern IDE:

- **Every window is a real OS window** — no custom window decorations, no dock widgets. Real OS windows on Wayland, Windows, and X11.
- **Browser-like tab management** — panels are tabs within a body. Drag a tab to tear off into a new window, or drop onto another body's edge to split.
- **Split bodies** — any body can be split horizontally or vertically into two resizable regions.
- **Auto-hiding tab bars** — the tab bar only appears when there is more than one panel in a body. With one panel, the bar disappears and the panel fills the space. A grab handle in the corner lets you summon the bar when needed.
- **Edge-anchored (pinned) panels** — panels can be anchored to any corner of the main window, showing a small icon in the side bar. Click to expand as an overlay or lock open as a splitter column/row.
- **Menu only on the main window** — torn-off windows have no menu. The menu bar belongs to the main editor window.
- **Layout presets** — save and restore workspace configurations as JSON.

---

## Architecture

### EditorWindow

Every panel derives from `EditorWindow`. Subclasses implement two things: `buildContent()` to populate their UI, and optionally `tabIcon()` to return a panel-specific icon.

```cpp
class SomePanel final : public Heathen::EditorWindow
{
public:
    SomePanel() : EditorWindow(QStringLiteral("Some Panel")) {}

    void buildContent() override
    {
        setBackgroundColor(QColor("#1e1e1e"));
        auto* layout = new QVBoxLayout(this);
        // ... add widgets ...
        setLayout(layout);
    }

    QIcon tabIcon() const override
    {
        return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    }
};
```

Register the panel type once at startup:

```cpp
EditorWindowRegistry::registerPanel({
    .name    = QStringLiteral("Some Panel"),
    .factory = []() -> EditorWindow* { return new SomePanel(); },
});
```

### EditorWindowContainer

A real OS window that holds one or more `EditorWindow` panels inside a tabbed, splittable body. The main window variant carries the menu bar and the pin system. Torn-off windows are plain containers without the menu or pin bars.

### Window Layout

```
┌────────────────────────── Main Window ─────────────────────────────┐
│  File   Settings   Panels   Tools                                  │
├──[ pin ]──────────┬───────────────────────┬──────────────[ pin ]───┤
│                   │                       │                        │
│  Entity Outliner  │      Scene View       │      Inspector         │
│                   │                       │                        │
│                   ├───────────────────────┤                        │
│                   │  Console  Asset Brow. │                        │
├───────────────────┴───────────────────────┴────────────────────────┤
│  O3DE UX-Mockup  v0.1.0                              [status]  [⊞] │
└────────────────────────────────────────────────────────────────────┘
```

### Prototype Panel Types

All panels are styled placeholders. The interaction system is real; the content is not.

| Panel | Category | Notes |
|---|---|---|
| Scene View | Viewport | `noClose`, `noPin` — always in the dock |
| Entity Outliner | Entities | |
| Inspector | Entities | Opens next to Entity Outliner by default |
| Asset Browser | Assets | Folder icon |
| Console | Tools | |
| Cool Script 1 | Scripts | Window icon; requires Cool Script 2 |
| Cool Script 2 | Scripts | Window icon |
| Main Menu UI | UI | |
| Friends List UI | UI | |
| Leaderboard UI | UI | Opens in a new window by default |
| Credits UI | UI | |

---

## Getting Involved

This is primarily for discussion with the O3DE community. [Share your thoughts in the O3DE Discord](https://discord.com/channels/805939474655346758/1493595628809359480).

---

## Status

| Feature | State |
|---|---|
| EditorWindow base class | Complete |
| EditorWindowContainer (real OS window) | Complete |
| Tab bar with drag, reorder, close | Complete |
| Auto-hiding tab bar + grab handle | Complete |
| Split bodies (horizontal / vertical) | Complete |
| Drag-to-split and drag-to-dock | Complete |
| Tear-off to floating window | Complete |
| Pin system (corner anchors, overlay, pinned-open) | Complete |
| Panel-defined tab icons | Complete |
| Status bar | Complete |
| Layout save / restore (JSON) | Complete |
| No-close / no-pin panel constraints | Complete |
| Required-panel relationships | Complete |
