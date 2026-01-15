#pragma once

#include <QFrame>

namespace cad {
namespace ui {

class QtViewport : public QFrame {
    Q_OBJECT

public:
    explicit QtViewport(QWidget* parent = nullptr);
};

}  // namespace ui
}  // namespace cad
