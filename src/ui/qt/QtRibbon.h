#pragma once

#include <functional>

#include <QTabWidget>

namespace cad {
namespace ui {

class QtRibbon : public QTabWidget {
    Q_OBJECT

public:
    explicit QtRibbon(QWidget* parent = nullptr);
    void setCommandHandler(const std::function<void(const QString&)>& handler);

private:
    QWidget* buildCommandTab(const QString& title, const QStringList& commands);

    std::function<void(const QString&)> command_handler_;
};

}  // namespace ui
}  // namespace cad
