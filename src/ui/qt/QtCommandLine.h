#pragma once

#include <QLineEdit>
#include <QStringList>

namespace cad {
namespace ui {

class QtCommandLine : public QLineEdit {
    Q_OBJECT

public:
    explicit QtCommandLine(QWidget* parent = nullptr);
    void setHistory(const QStringList& history);
    const QStringList& history() const;
    QString takeCurrentCommand();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    QStringList history_{};
    int history_index_{-1};
};

}  // namespace ui
}  // namespace cad
