#include "QtRibbon.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace cad {
namespace ui {

QtRibbon::QtRibbon(QWidget* parent) : QTabWidget(parent) {
    addTab(buildCommandTab(tr("Sketch"),
                           {tr("Line"), tr("Rectangle"), tr("Circle"), tr("Arc"), tr("Constraint")}),
           tr("Sketch"));
    addTab(buildCommandTab(tr("Part"),
                           {tr("Extrude"), tr("Revolve"), tr("Loft"), tr("Hole"), tr("Fillet"),
                            tr("Flange"), tr("Bend"), tr("Unfold"), tr("Refold"),
                            tr("Rectangular Pattern"), tr("Circular Pattern"),
                            tr("Curve Pattern"), tr("Direct Edit"), tr("Freeform")},
           tr("Part"));
    addTab(buildCommandTab(tr("Assembly"),
                           {tr("Place"), tr("Mate"), tr("Flush"), tr("Angle"), tr("Pattern"),
                            tr("Rigid Pipe"), tr("Flexible Hose"), tr("Bent Tube"),
                            tr("Simplify")}),
           tr("Assembly"));
    addTab(buildCommandTab(tr("Drawing"),
                           {tr("Base View"), tr("Section"), tr("Dimension"), tr("Parts List")}),
           tr("Drawing"));
    addTab(buildCommandTab(tr("Inspect"),
                           {tr("Measure"), tr("Interference"), tr("Section Analysis"),
                            tr("Simulation"), tr("Stress Analysis")}),
           tr("Inspect"));
    addTab(buildCommandTab(tr("Manage"),
                           {tr("Parameters"), tr("Styles"), tr("Add-Ins"), tr("Import"),
                            tr("Export"), tr("Export RFA"), tr("MBD Note")}),
           tr("Manage"));
    addTab(buildCommandTab(tr("View"),
                           {tr("Visibility"), tr("Appearance"), tr("Environment"),
                            tr("Illustration"), tr("Rendering"), tr("Animation")}),
           tr("View"));
}

void QtRibbon::setCommandHandler(const std::function<void(const QString&)>& handler) {
    command_handler_ = handler;
}

QWidget* QtRibbon::buildCommandTab(const QString& title, const QStringList& commands) {
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->addWidget(new QLabel(title));
    for (const auto& command : commands) {
        QPushButton* button = new QPushButton(command, page);
        if (command_handler_) {
            QObject::connect(button, &QPushButton::clicked, this, [this, command]() {
                if (command_handler_) {
                    command_handler_(command);
                }
            });
        }
        layout->addWidget(button);
    }
    layout->addStretch();
    return page;
}

}  // namespace ui
}  // namespace cad
