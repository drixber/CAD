#include "QtPropertyPanel.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

namespace cad {
namespace ui {

QtPropertyPanel::QtPropertyPanel(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Selection")));
    constraints_label_ = new QLabel(tr("Constraints: 0"));
    layout->addWidget(constraints_label_);
    parameters_label_ = new QLabel(tr("Parameters: 0"));
    layout->addWidget(parameters_label_);
    parameters_summary_ = new QLabel(tr("Parameter Summary:"));
    parameters_summary_->setWordWrap(true);
    layout->addWidget(parameters_summary_);
    integration_status_ = new QLabel(tr("Integration: FreeCAD off"));
    layout->addWidget(integration_status_);
    mates_label_ = new QLabel(tr("Mates: 0"));
    layout->addWidget(mates_label_);
    context_label_ = new QLabel(tr("Context: None"));
    layout->addWidget(context_label_);

    context_stack_ = new QStackedWidget(this);
    auto makePanel = [this](const QString& title, const QStringList& fields) {
        QWidget* panel = new QWidget(context_stack_);
        QVBoxLayout* panelLayout = new QVBoxLayout(panel);
        QLabel* header = new QLabel(title, panel);
        header->setAlignment(Qt::AlignLeft);
        panelLayout->addWidget(header);
        QFormLayout* form = new QFormLayout();
        for (const auto& field : fields) {
            QLineEdit* input = new QLineEdit(panel);
            input->setPlaceholderText(field);
            form->addRow(field + ":", input);
        }
        panelLayout->addLayout(form);
        panelLayout->addStretch();
        panel->setLayout(panelLayout);
        return panel;
    };

    context_stack_->addWidget(makePanel(tr("Context Panel: General"),
                                        {tr("Name"), tr("Description")}));
    context_stack_->addWidget(makePanel(tr("Context Panel: Sketch"),
                                        {tr("Constraint"), tr("Dimension")}));
    context_stack_->addWidget(makePanel(tr("Context Panel: Part"),
                                        {tr("Feature"), tr("Depth")}));
    context_stack_->addWidget(makePanel(tr("Context Panel: Assembly"),
                                        {tr("Mate Type"), tr("Offset")}));
    context_stack_->addWidget(makePanel(tr("Context Panel: Drawing"),
                                        {tr("View Type"), tr("Scale")}));
    context_stack_->addWidget(makePanel(tr("Context Panel: Inspect"),
                                        {tr("Tool"), tr("Result")}));
    context_stack_->addWidget(makePanel(tr("Context Panel: Manage"),
                                        {tr("Parameter"), tr("Value")}));
    context_stack_->addWidget(makePanel(tr("Context Panel: View"),
                                        {tr("Appearance"), tr("Environment")}));
    layout->addWidget(context_stack_);

    layout->addStretch();
}

void QtPropertyPanel::setConstraintCount(int count) {
    if (constraints_label_) {
        constraints_label_->setText(tr("Constraints: %1").arg(count));
    }
}

void QtPropertyPanel::setParameterCount(int count) {
    if (parameters_label_) {
        parameters_label_->setText(tr("Parameters: %1").arg(count));
    }
}

void QtPropertyPanel::setParameterSummary(const QString& summary) {
    if (parameters_summary_) {
        parameters_summary_->setText(tr("Parameter Summary: %1").arg(summary));
    }
}

void QtPropertyPanel::setIntegrationStatus(const QString& status) {
    if (integration_status_) {
        integration_status_->setText(tr("Integration: %1").arg(status));
    }
}

void QtPropertyPanel::setMateCount(int count) {
    if (mates_label_) {
        mates_label_->setText(tr("Mates: %1").arg(count));
    }
}

void QtPropertyPanel::setContextPlaceholder(const QString& context) {
    if (context_label_) {
        context_label_->setText(tr("Context: %1").arg(context));
    }
}

void QtPropertyPanel::setContextCategory(const QString& category) {
    if (!context_stack_) {
        return;
    }
    const QString normalized = category.toLower();
    int index = 0;
    if (normalized == "sketch") {
        index = 1;
    } else if (normalized == "part") {
        index = 2;
    } else if (normalized == "assembly") {
        index = 3;
    } else if (normalized == "drawing") {
        index = 4;
    } else if (normalized == "inspect") {
        index = 5;
    } else if (normalized == "manage") {
        index = 6;
    } else if (normalized == "view") {
        index = 7;
    }
    context_stack_->setCurrentIndex(index);
}

}  // namespace ui
}  // namespace cad
