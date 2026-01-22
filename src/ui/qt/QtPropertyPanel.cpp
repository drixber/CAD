#include "QtPropertyPanel.h"

#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
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
    // Drawing panel with BOM table
    QWidget* drawing_panel = new QWidget(context_stack_);
    QVBoxLayout* drawing_layout = new QVBoxLayout(drawing_panel);
    QLabel* drawing_header = new QLabel(tr("Context Panel: Drawing"), drawing_panel);
    drawing_header->setAlignment(Qt::AlignLeft);
    drawing_layout->addWidget(drawing_header);
    
    QFormLayout* drawing_form = new QFormLayout();
    QLineEdit* view_type = new QLineEdit(drawing_panel);
    view_type->setPlaceholderText(tr("View Type"));
    drawing_form->addRow(tr("View Type:"), view_type);
    QLineEdit* scale = new QLineEdit(drawing_panel);
    scale->setPlaceholderText(tr("Scale"));
    drawing_form->addRow(tr("Scale:"), scale);
    QLineEdit* sheet = new QLineEdit(drawing_panel);
    sheet->setPlaceholderText(tr("Sheet"));
    drawing_form->addRow(tr("Sheet:"), sheet);
    drawing_layout->addLayout(drawing_form);
    
    QLabel* bom_label = new QLabel(tr("Bill of Materials:"), drawing_panel);
    drawing_layout->addWidget(bom_label);
    
    bom_table_ = new QTableWidget(drawing_panel);
    bom_table_->setColumnCount(3);
    bom_table_->setHorizontalHeaderLabels({tr("Part Name"), tr("Quantity"), tr("Part Number")});
    bom_table_->horizontalHeader()->setStretchLastSection(true);
    bom_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bom_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    bom_table_->setAlternatingRowColors(true);
    bom_table_->setMaximumHeight(200);
    drawing_layout->addWidget(bom_table_);
    
    drawing_layout->addStretch();
    drawing_panel->setLayout(drawing_layout);
    context_stack_->addWidget(drawing_panel);
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

void QtPropertyPanel::setBomItems(const QList<BomItem>& items) {
    updateBomTable(items);
}

void QtPropertyPanel::updateBomTable(const QList<BomItem>& items) {
    if (!bom_table_) {
        return;
    }
    
    bom_table_->setRowCount(items.size());
    for (int i = 0; i < items.size(); ++i) {
        const BomItem& item = items[i];
        bom_table_->setItem(i, 0, new QTableWidgetItem(item.part_name));
        bom_table_->setItem(i, 1, new QTableWidgetItem(QString::number(item.quantity)));
        bom_table_->setItem(i, 2, new QTableWidgetItem(item.part_number));
    }
    
    bom_table_->resizeColumnsToContents();
}

}  // namespace ui
}  // namespace cad
