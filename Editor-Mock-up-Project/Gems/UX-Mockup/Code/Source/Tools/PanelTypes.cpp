#include "PanelTypes.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QVBoxLayout>

namespace {

// Viewport background widget with a dark grid overlay.
class GridWidget : public QWidget
{
public:
    explicit GridWidget(QWidget* parent = nullptr) : QWidget(parent)
    {
        setAttribute(Qt::WA_OpaquePaintEvent);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.fillRect(rect(), QColor("#1a1a1a"));

        p.setPen(QPen(QColor("#242424"), 1));
        for (int x = 0; x < width();  x += 40) p.drawLine(x, 0, x, height());
        for (int y = 0; y < height(); y += 40) p.drawLine(0, y, width(), y);

        p.setPen(QPen(QColor("#303030"), 1));
        p.drawLine(width() / 2, 0,         width() / 2, height());
        p.drawLine(0,           height()/2, width(),     height() / 2);

        p.setPen(QColor("#3a3a3a"));
        p.setFont(QFont("Arial", 14, QFont::Bold));
        p.drawText(rect(), Qt::AlignCenter, "3D Viewport");
    }
};

// ---- layout helpers --------------------------------------------------------

QWidget* makeHeader(const QString& title, QWidget* parent)
{
    auto* bar = new QWidget(parent);
    bar->setFixedHeight(28);
    bar->setStyleSheet("background-color: #2d2d2d; border-bottom: 1px solid #3c3c3c;");

    auto* label = new QLabel(title.toUpper(), bar);
    label->setStyleSheet(
        "color: #9a9a9a; font-size: 10px; font-weight: bold; padding-left: 10px;");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    auto* lay = new QHBoxLayout(bar);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(label);
    return bar;
}

QLabel* makeRow(QWidget* parent, const QString& text, int indent = 0)
{
    auto* lbl = new QLabel(text, parent);
    lbl->setStyleSheet(
        QString("color: #969696; font-size: 12px;"
                " padding: 2px 8px 2px %1px; background: transparent;")
            .arg(8 + indent * 16));
    lbl->setFixedHeight(22);
    return lbl;
}

QLabel* makePropRow(QWidget* parent, const QString& name, const QString& value)
{
    auto* lbl = new QLabel(parent);
    lbl->setText(QString("<span style='color:#7a7a7a'>%1</span>"
                         "<span style='color:#cccccc'> %2</span>")
                     .arg(name, value));
    lbl->setStyleSheet("font-size: 12px; padding: 2px 8px; background: transparent;");
    lbl->setFixedHeight(22);
    return lbl;
}

QWidget* makeSep(QWidget* parent)
{
    auto* w = new QWidget(parent);
    w->setFixedHeight(1);
    w->setStyleSheet("background-color: #2d2d2d;");
    return w;
}

} // anonymous namespace

// ============================================================================
namespace Heathen
{

// --- Scene View -------------------------------------------------------------

SceneViewPanel::SceneViewPanel()
    : EditorWindow(QStringLiteral("Scene View")) {}

void SceneViewPanel::buildContent()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(new GridWidget(this));
    setLayout(layout);
}

// --- Entity Outliner --------------------------------------------------------

EntityOutliner::EntityOutliner()
    : EditorWindow(QStringLiteral("Entity Outliner")) {}

void EntityOutliner::buildContent()
{
    setBackgroundColor(QColor("#1e1e1e"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(makeHeader("Entity Outliner", this));
    layout->addWidget(makeRow(this, "▸  World"));
    layout->addWidget(makeRow(this, "▸  Player",      1));
    layout->addWidget(makeRow(this, "   Camera",           2));
    layout->addWidget(makeRow(this, "▸  Environment", 1));
    layout->addWidget(makeRow(this, "   Sky",              2));
    layout->addWidget(makeRow(this, "   Terrain",          2));
    layout->addWidget(makeRow(this, "▸  Lights",      1));
    layout->addWidget(makeSep(this));
    layout->addStretch(1);
    setLayout(layout);
}

// --- Inspector --------------------------------------------------------------

InspectorPanel::InspectorPanel()
    : EditorWindow(QStringLiteral("Inspector")) {}

void InspectorPanel::buildContent()
{
    setBackgroundColor(QColor("#1e1e1e"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(makeHeader("Inspector", this));
    layout->addWidget(makePropRow(this, "Name",       "Player"));
    layout->addWidget(makeSep(this));
    layout->addWidget(makePropRow(this, "Transform",  ""));
    layout->addWidget(makePropRow(this, "  Position", "0.0, 1.8, 0.0"));
    layout->addWidget(makePropRow(this, "  Rotation", "0°, 0°, 0°"));
    layout->addWidget(makePropRow(this, "  Scale",    "1.0, 1.0, 1.0"));
    layout->addWidget(makeSep(this));
    layout->addWidget(makePropRow(this, "Physics",    ""));
    layout->addWidget(makePropRow(this, "  Mass",     "70 kg"));
    layout->addWidget(makePropRow(this, "  Friction", "0.5"));
    layout->addWidget(makeSep(this));
    layout->addStretch(1);
    setLayout(layout);
}

// --- Asset Browser ----------------------------------------------------------

AssetBrowserPanel::AssetBrowserPanel()
    : EditorWindow(QStringLiteral("Asset Browser")) {}

void AssetBrowserPanel::buildContent()
{
    setBackgroundColor(QColor("#1e1e1e"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(makeHeader("Asset Browser", this));

    // Column header bar.
    auto* cols = new QWidget(this);
    cols->setFixedHeight(22);
    cols->setStyleSheet("background-color: #252526; border-bottom: 1px solid #3c3c3c;");
    auto* cl = new QHBoxLayout(cols);
    cl->setContentsMargins(8, 0, 8, 0);
    for (const QString& h : {"Name", "Type", "Size"})
    {
        auto* lbl = new QLabel(h, cols);
        lbl->setStyleSheet("color: #7a7a7a; font-size: 11px;");
        cl->addWidget(lbl, h == "Name" ? 3 : 1);
    }
    layout->addWidget(cols);

    struct Row { QString name, type, size; };
    static const Row rows[] = {
        {"player.fbx",   "Mesh",     "1.2 MB"},
        {"ground.mat",   "Material", "12 KB"},
        {"sky.azshader", "Shader",   "48 KB"},
        {"music.ogg",    "Audio",    "2.8 MB"},
        {"tree.prefab",  "Prefab",   "4 KB"},
    };
    for (const auto& r : rows)
    {
        auto* row = new QWidget(this);
        row->setFixedHeight(22);
        auto* rl = new QHBoxLayout(row);
        rl->setContentsMargins(8, 0, 8, 0);

        auto* n = new QLabel(r.name, row); n->setStyleSheet("color: #cccccc; font-size: 12px;");
        auto* t = new QLabel(r.type, row); t->setStyleSheet("color: #7a7a7a; font-size: 12px;");
        auto* s = new QLabel(r.size, row); s->setStyleSheet("color: #7a7a7a; font-size: 12px;");
        rl->addWidget(n, 3); rl->addWidget(t, 1); rl->addWidget(s, 1);

        layout->addWidget(row);
        layout->addWidget(makeSep(this));
    }
    layout->addStretch(1);
    setLayout(layout);
}

QIcon AssetBrowserPanel::tabIcon() const
{
    return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
}

// --- Console ----------------------------------------------------------------

ConsolePanel::ConsolePanel()
    : EditorWindow(QStringLiteral("Console")) {}

void ConsolePanel::buildContent()
{
    setBackgroundColor(QColor("#0c0c0c"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(makeHeader("Output Console", this));

    struct Entry { QString text; QString color; };
    static const Entry entries[] = {
        {"[INFO]  Engine initialised — build 1.0.0-prototype", "#4a9a4a"},
        {"[INFO]  Gem loaded: UX-Mockup",                           "#4a9a4a"},
        {"[INFO]  Main window created",                             "#4a9a4a"},
        {"[WARN]  Placeholder panels: no real engine content",      "#9a9a2a"},
        {"[INFO]  Ready.",                                          "#4a9a4a"},
    };
    for (const auto& e : entries)
    {
        auto* lbl = new QLabel(e.text, this);
        lbl->setStyleSheet(
            QString("color: %1; font-family: Consolas, monospace;"
                    " font-size: 12px; padding: 1px 8px;").arg(e.color));
        lbl->setFixedHeight(20);
        layout->addWidget(lbl);
    }
    layout->addStretch(1);
    setLayout(layout);
}

// --- Cool Script 1 ----------------------------------------------------------

SomeCoolScript::SomeCoolScript()
    : EditorWindow(QStringLiteral("Cool Script 1")) {}

void SomeCoolScript::buildContent()
{
    setBackgroundColor(QColor("#131a22"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(makeHeader("Cool Script 1", this));

    auto* hint = new QLabel("[ Canvas — dock this next to Cool Script 2 to see two-script layout ]", this);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: #2a4a6a; font-size: 12px;");
    layout->addWidget(hint, 1);
    setLayout(layout);
}

QIcon SomeCoolScript::tabIcon() const
{
    return QApplication::style()->standardIcon(QStyle::SP_TitleBarMaxButton);
}

// --- Cool Script 2 ----------------------------------------------------------

AnotherCoolScript::AnotherCoolScript()
    : EditorWindow(QStringLiteral("Cool Script 2")) {}

void AnotherCoolScript::buildContent()
{
    setBackgroundColor(QColor("#13221a"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(makeHeader("Cool Script 2", this));

    auto* hint = new QLabel("[ Canvas — opens automatically when Cool Script 1 is opened ]", this);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: #2a5a3a; font-size: 12px;");
    layout->addWidget(hint, 1);
    setLayout(layout);
}

QIcon AnotherCoolScript::tabIcon() const
{
    return QApplication::style()->standardIcon(QStyle::SP_TitleBarMaxButton);
}

// --- Main Menu UI -----------------------------------------------------------

MainMenuUI::MainMenuUI()
    : EditorWindow(QStringLiteral("Main Menu UI")) {}

void MainMenuUI::buildContent()
{
    setBackgroundColor(QColor("#1e1e2a"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(makeHeader("Main Menu UI", this));
    layout->addSpacing(20);

    for (const QString& item : {"New Game", "Load Game", "Settings", "Credits", "Exit"})
    {
        auto* btn = new QLabel(item, this);
        btn->setAlignment(Qt::AlignCenter);
        btn->setFixedHeight(38);
        btn->setStyleSheet(
            "color: #cccccc; font-size: 13px;"
            " border: 1px solid #3a3a5a; margin: 3px 24px;");
        layout->addWidget(btn);
    }
    layout->addStretch(1);
    setLayout(layout);
}

// --- Friends List UI --------------------------------------------------------

FriendsListUI::FriendsListUI()
    : EditorWindow(QStringLiteral("Friends List UI")) {}

void FriendsListUI::buildContent()
{
    setBackgroundColor(QColor("#1a1e24"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(makeHeader("Friends List UI", this));

    struct Friend { QString name, status, color; };
    static const Friend friends[] = {
        {"Alice",   "Online",  "#4a9a4a"},
        {"Bob",     "In Game", "#4a7a9a"},
        {"Charlie", "Away",    "#9a9a2a"},
        {"Dave",    "Offline", "#5a5a5a"},
    };
    for (const auto& f : friends)
    {
        auto* row = new QWidget(this);
        row->setFixedHeight(32);
        auto* rl = new QHBoxLayout(row);
        rl->setContentsMargins(12, 0, 12, 0);

        auto* name   = new QLabel(f.name, row);
        name->setStyleSheet("color: #cccccc; font-size: 13px;");

        auto* status = new QLabel(f.status, row);
        status->setStyleSheet(QString("color: %1; font-size: 11px;").arg(f.color));
        status->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        rl->addWidget(name);
        rl->addWidget(status);
        layout->addWidget(row);
        layout->addWidget(makeSep(this));
    }
    layout->addStretch(1);
    setLayout(layout);
}

// --- Leaderboard UI ---------------------------------------------------------

LeaderboardUI::LeaderboardUI()
    : EditorWindow(QStringLiteral("Leaderboard UI")) {}

void LeaderboardUI::buildContent()
{
    setBackgroundColor(QColor("#1e1a24"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(makeHeader("Leaderboard UI", this));

    struct Entry { int rank; QString name; int score; };
    static const Entry entries[] = {
        {1, "Alice",   98500},
        {2, "Bob",     87200},
        {3, "Charlie", 75000},
        {4, "Dave",    62100},
        {5, "Eve",     51300},
    };
    for (const auto& e : entries)
    {
        const QString rankColor = e.rank == 1 ? "#c8a020" :
                                  e.rank == 2 ? "#9a9a9a" :
                                  e.rank == 3 ? "#c07840" : "#555555";

        auto* row = new QWidget(this);
        row->setFixedHeight(28);
        auto* rl = new QHBoxLayout(row);
        rl->setContentsMargins(12, 0, 12, 0);

        auto* rank  = new QLabel(QString("#%1").arg(e.rank), row);
        rank->setStyleSheet(
            QString("color: %1; font-size: 12px; font-weight: bold;").arg(rankColor));
        rank->setFixedWidth(30);

        auto* name  = new QLabel(e.name, row);
        name->setStyleSheet("color: #cccccc; font-size: 13px;");

        auto* score = new QLabel(QString::number(e.score), row);
        score->setStyleSheet("color: #9a9a9a; font-size: 12px;");
        score->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        rl->addWidget(rank);
        rl->addWidget(name);
        rl->addWidget(score);
        layout->addWidget(row);
        layout->addWidget(makeSep(this));
    }
    layout->addStretch(1);
    setLayout(layout);
}

// --- Credits UI -------------------------------------------------------------

CreditsUI::CreditsUI()
    : EditorWindow(QStringLiteral("Credits UI")) {}

void CreditsUI::buildContent()
{
    setBackgroundColor(QColor("#0a0a14"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(makeHeader("Credits UI", this));
    layout->addSpacing(24);

    const auto addCredit = [&](const QString& role, const QString& name)
    {
        auto* lbl = new QLabel(this);
        lbl->setText(
            QString("<div style='text-align:center;'>"
                    "<span style='color:#4a4a6a;font-size:9px;'>%1</span><br>"
                    "<span style='color:#cccccc;font-size:13px;font-weight:bold;'>%2</span>"
                    "</div>")
                .arg(role, name));
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setContentsMargins(0, 4, 0, 12);
        layout->addWidget(lbl);
    };

    addCredit("UX DESIGN",     "James McGhee");
    addCredit("ENGINE",        "O3DE Foundation");
    addCredit("PROTOTYPE GEM", "UX-Mockup v0.1");
    addCredit("FRAMEWORK",     "Qt 6.x");

    layout->addStretch(1);
    setLayout(layout);
}

} // namespace Heathen
