# Pin System — Corner Anchoring

## Original Notes
This is for context and reference only. Sse the sections below for design:

- We need to rethink the anchoring system points follow
  -- anchoring should attach to the corners of the window so lets take on the same model as Clion/Rider ... instead of right click [Anchor] lets give a right click [Pin] but this expands into a submenu with the options Bottom Left, Bottom Right, Left Top, Left Bottom, Right Top, Right Bottom.
  -- Doing this removes the tab from the container and "pins" it to the window border in that respective corner. only its Icon shows, when clicked it expands open from that edge of the window, unlike CLion/Rider this is rendering overtop of the window,
  -- the "tab" or "header" should have a little pin icon if clicked it will remain open and more like Clion/Rider does. That is a Left or Rigth Top expands from the left or right side of the window, a Bottom Left or Right expands from the bottom of the window. these can be pinned open causing them to act as panels in the main window but are not part of the tab dock system.

For example if I was to have all 4 options open and pinned it form a U shape around the left, lower and right edge of the screen, the valid tab dock area then in the space in teh upper middle of that U only ... this would look like this
 
```
------------------------------------------------------------------
|  Top Left Pins   |      Tab Dock Area     |  Top Right Pins    |
|                  |                        |                    |
|                  |                        |                    |
 ------------------------------------------------------------------
|                                |                               |
|                                |                               |
|           Bottom Left Pins     |      Bottom Right Pins        |
 ------------------------------------------------------------------
```

The borders between the pins and the dock areas and the pins with each other such as the left right bottom or the bottom/top borders is resizable and determins the size a panel would expand to when expanded but not pinned
-- When a pinned panel is pinned open the pin icon on the tab/header can be clicked again causing it to resume the collapsable behaivour so clicking anywhere off its body will cause it to collapse back to its icons home.
-- While the header does expand out with it the icon of a pinned panel remains on the corner/edge of the main window an is highlighted when its open, this works like a Tab page indicating which of the pinned panels is currently displayed if anywhere
-- A panel can be right clicked and "Unpinned" causing it to become a Tab Dock again whcih can be draged into the dock area or used to split panels or left as a floating window of its own
-- Pinned panels only pin to the main window, so selecting "Pin > Top Right" will make an icon along the right edge of the main window at the top and all Top Right pinned panels are there stacked top to botttom
-- As to icon locations Top Right stacks along the right edge from the top to the bottom, Top Left stacks the same but on the left edge, Bottom Left and Bottom Right stack on the left or right edge but starting at the bottom and going up

## Core Concept
Replace edge-collapse anchoring with corner-pinned panels (Clion/Rider model). Each pin is a corner of the main window. Each corner holds 1 panel (tab-page model — clicking a different panel replaces the current one).

## Pin Positions

| Pin | Icon location | Expand direction |
|-----|--------------|--|
| TopLeft | Left edge, top | From left edge |
| TopRight | Right edge, top | From right edge |
| BottomLeft | Left edge, bottom | From bottom edge |
|BottomRight | Right edge, bottom | From bottom edge |

## Layout When Multiple Pins Open

Pinned panels expand outward from their edge. Dock area is always a rectangle — remaining space.
Examples
```
Let 
- TL = Top Left,
- TR = Top Right,
- BL = Bottom Left,
- BR = Bottom Right.

TL open, none others:
┌─────┬────────────────────────┐
│     │                        │
│ TL  │   Dock Area            │
│     │                        │
│     │                        │
└─────┴────────────────────────┘

TL + TR
┌───────┬──────────────┬───────┐
│ TL    │              │ TR    │
│       │              │       │
│       │ Dock Area    │       │
│       │              │       │
└───────┴──────────────┴───────┘

TL + BL + TR + BR all open (L-shape):
┌───────┬──────────────┬───────┐
│ TL    │              │ TR    │
│       │              │       │
│       │ Dock Area    │       │
│       │              │       │
├───────└───────┬──────┴───────┤
│ BL            │ BR           │
└───────────────┴──────────────┘

TR + BL
┌──────────────────────┬───────┐
│                      │ TR    │
│                      │       │
│       Dock Area      │       │
│                      │       │
├──────────────────────┴───────┤
│ BL                           │
└──────────────────────────────┘

TR + BR
┌──────────────────────┬───────┐
│                      │ TR    │
│                      │       │
│       Dock Area      │       │
│                      │       │
├──────────────────────┴───────┤
│ BR                           │
└──────────────────────────────┘

TL + BL
┌──────┬───────────────────────┐
│ TL   │                       │
│      │                       │
│      │Dock Area              │
│      │                       │
├──────┴───────────────────────┤
│ BL                           │
└──────────────────────────────┘

TL + BR
┌──────┬───────────────────────┐
│ TL   │                       │
│      │                       │
│      │Dock Area              │
│      │                       │
├──────┴───────────────────────┤
│ BR                           │
└──────────────────────────────┘

TR + BL
┌──────────────────────┬───────┐
│                      │ TR    │
│                      │       │
│       Dock Area      │       │
│                      │       │
├──────────────────────┴───────┤
│ BL                           │
└──────────────────────────────┘

TR + BR + BL
┌──────────────────────┬───────┐
│                      │ TR    │
│                      │       │
│       Dock Area      │       │
│                      │       │
├───────────────┬──────┴───────┤
│ BL            │ BR           │
└───────────────┴──────────────┘

TL + BR + BL
┌──────┬───────────────────────┐
│ TL   │                       │
│      │                       │
│      │Dock Area              │
│      │                       │
├──────└────────┬──────────────┤
│ BL            │ BR           │
└───────────────┴──────────────┘
```

- BL + BR open → split bottom edge 50/50 (resizable)
- TL + BL open → split left edge 50/50 (resizable)
- TL + BR open → split left edge 50/50 (resizable)
- All 4 → U-shape, dock area fills rectangular interior
- Default equal split; user drags borders to resize, this should be saved in user preferences and layout settings

## Two States Per Pinned Panel

**Collapsed (icon only):**
- Only the tab's icon visible on window edge bars
- Click icon → panel overlays window content (on top, no layout change) expanding out to a size similar to what would be pinned open
- Click off → collapses to icon
- Takes NO layout space used this is an overlay

**Pinned open (layout participant):**
- Panel takes real layout space
- Pushes dock area inward
- Toggle on/off via pin icon on the tab/header of the panel
- Takes a slice of assigned edge

## Context Menu

Every panel tab has right-click → Move To:
- Pin Top Left
- Pin Top Right
- Pin Bottom Left
- Pin Bottom Right
- Dockable 

Dockable is the state of our current windows that is it can be docked or used to split the dock area, and it can be made a window of its own by droping it outside of a dockable area

Current position checked/highlighted. Moving to a corner with existing panel replaces it.

## Icon Highlighting
Icons for the pinned panels on corner edge always visible. Highlights when its panel is displayed — acts as tab page indicator for which pinned panel is active.
