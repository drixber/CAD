#include "QtPropertyPanel.h"

#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
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
    
    // BOM filter and sort controls
    QHBoxLayout* bom_controls = new QHBoxLayout();
    bom_filter_ = new QLineEdit(drawing_panel);
    bom_filter_->setPlaceholderText(tr("Filter..."));
    bom_filter_->setClearButtonEnabled(true);
    bom_controls->addWidget(bom_filter_);
    
    bom_sort_column_ = new QComboBox(drawing_panel);
    bom_sort_column_->addItems({tr("Part Name"), tr("Quantity"), tr("Part Number")});
    bom_controls->addWidget(bom_sort_column_);
    
    bom_export_button_ = new QPushButton(tr("Export"), drawing_panel);
    bom_controls->addWidget(bom_export_button_);
    drawing_layout->addLayout(bom_controls);
    
    bom_table_ = new QTableWidget(drawing_panel);
    bom_table_->setColumnCount(3);
    bom_table_->setHorizontalHeaderLabels({tr("Part Name"), tr("Quantity"), tr("Part Number")});
    bom_table_->horizontalHeader()->setStretchLastSection(true);
    bom_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bom_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    bom_table_->setAlternatingRowColors(true);
    bom_table_->setSortingEnabled(true);
    bom_table_->setMaximumHeight(200);
    drawing_layout->addWidget(bom_table_);
    
    // Connect signals
    connect(bom_filter_, &QLineEdit::textChanged, this, [this]() { filterBomTable(); });
    connect(bom_sort_column_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { sortBomTable(); });
    connect(bom_export_button_, &QPushButton::clicked, this, [this]() { exportBomTable(); });
    
    QLabel* annotation_label = new QLabel(tr("Annotations:"), drawing_panel);
    drawing_layout->addWidget(annotation_label);
    
    annotation_table_ = new QTableWidget(drawing_panel);
    annotation_table_->setColumnCount(5);
    annotation_table_->setHorizontalHeaderLabels({tr("Text"), tr("Type"), tr("X"), tr("Y"), tr("Leader")});
    annotation_table_->horizontalHeader()->setStretchLastSection(true);
    annotation_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    annotation_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    annotation_table_->setAlternatingRowColors(true);
    annotation_table_->setMaximumHeight(200);
    drawing_layout->addWidget(annotation_table_);
    
    drawing_layout->addStretch();
    drawing_panel->setLayout(drawing_layout);
    context_stack_->addWidget(drawing_panel);
    context_stack_->addWidget(makePanel(tr("Context Panel: Inspect"),
                                        {tr("Tool"), tr("Result")}));
    
    // Manage panel with style editor
    QWidget* manage_panel = new QWidget(context_stack_);
    QVBoxLayout* manage_layout = new QVBoxLayout(manage_panel);
    QLabel* manage_header = new QLabel(tr("Context Panel: Manage"), manage_panel);
    manage_header->setAlignment(Qt::AlignLeft);
    manage_layout->addWidget(manage_header);
    
    QFormLayout* manage_form = new QFormLayout();
    QLineEdit* parameter = new QLineEdit(manage_panel);
    parameter->setPlaceholderText(tr("Parameter"));
    manage_form->addRow(tr("Parameter:"), parameter);
    QLineEdit* value = new QLineEdit(manage_panel);
    value->setPlaceholderText(tr("Value"));
    manage_form->addRow(tr("Value:"), value);
    manage_layout->addLayout(manage_form);
    
    QLabel* style_label = new QLabel(tr("Drawing Styles:"), manage_panel);
    manage_layout->addWidget(style_label);
    
    QHBoxLayout* preset_layout = new QHBoxLayout();
    preset_layout->addWidget(new QLabel(tr("Preset:"), manage_panel));
    style_preset_selector_ = new QComboBox(manage_panel);
    style_preset_selector_->addItems({tr("Default"), tr("ISO"), tr("ANSI"), tr("JIS")});
    preset_layout->addWidget(style_preset_selector_);
    manage_layout->addLayout(preset_layout);
    
    style_preset_label_ = new QLabel(tr("Current: Default"), manage_panel);
    manage_layout->addWidget(style_preset_label_);
    
    style_info_label_ = new QLabel(tr("Line styles: 0, Text styles: 0, Dimension styles: 0"), manage_panel);
    style_info_label_->setWordWrap(true);
    manage_layout->addWidget(style_info_label_);
    
    // Line styles table
    QLabel* line_styles_label = new QLabel(tr("Line Styles:"), manage_panel);
    manage_layout->addWidget(line_styles_label);
    
    line_styles_table_ = new QTableWidget(manage_panel);
    line_styles_table_->setColumnCount(4);
    line_styles_table_->setHorizontalHeaderLabels({tr("Name"), tr("Thickness"), tr("Type"), tr("Color")});
    line_styles_table_->horizontalHeader()->setStretchLastSection(true);
    line_styles_table_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    line_styles_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    line_styles_table_->setAlternatingRowColors(true);
    line_styles_table_->setMaximumHeight(150);
    setupStyleTableEditing(line_styles_table_);
    manage_layout->addWidget(line_styles_table_);
    
    // Text styles table
    QLabel* text_styles_label = new QLabel(tr("Text Styles:"), manage_panel);
    manage_layout->addWidget(text_styles_label);
    
    text_styles_table_ = new QTableWidget(manage_panel);
    text_styles_table_->setColumnCount(4);
    text_styles_table_->setHorizontalHeaderLabels({tr("Name"), tr("Size"), tr("Font"), tr("Weight")});
    text_styles_table_->horizontalHeader()->setStretchLastSection(true);
    text_styles_table_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    text_styles_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    text_styles_table_->setAlternatingRowColors(true);
    text_styles_table_->setMaximumHeight(150);
    setupStyleTableEditing(text_styles_table_);
    manage_layout->addWidget(text_styles_table_);
    
    // Dimension styles table
    QLabel* dimension_styles_label = new QLabel(tr("Dimension Styles:"), manage_panel);
    manage_layout->addWidget(dimension_styles_label);
    
    dimension_styles_table_ = new QTableWidget(manage_panel);
    dimension_styles_table_->setColumnCount(5);
    dimension_styles_table_->setHorizontalHeaderLabels({tr("Name"), tr("Text Height"), tr("Arrow Size"), tr("Units"), tr("Color")});
    dimension_styles_table_->horizontalHeader()->setStretchLastSection(true);
    dimension_styles_table_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    dimension_styles_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    dimension_styles_table_->setAlternatingRowColors(true);
    dimension_styles_table_->setMaximumHeight(150);
    setupStyleTableEditing(dimension_styles_table_);
    manage_layout->addWidget(dimension_styles_table_);
    
    // Hatch styles table
    QLabel* hatch_styles_label = new QLabel(tr("Hatch Styles:"), manage_panel);
    manage_layout->addWidget(hatch_styles_label);
    
    hatch_styles_table_ = new QTableWidget(manage_panel);
    hatch_styles_table_->setColumnCount(4);
    hatch_styles_table_->setHorizontalHeaderLabels({tr("Name"), tr("Pattern"), tr("Scale"), tr("Color")});
    hatch_styles_table_->horizontalHeader()->setStretchLastSection(true);
    hatch_styles_table_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    hatch_styles_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    hatch_styles_table_->setAlternatingRowColors(true);
    hatch_styles_table_->setMaximumHeight(150);
    setupStyleTableEditing(hatch_styles_table_);
    manage_layout->addWidget(hatch_styles_table_);
    
    // Style preview widget
    QLabel* preview_label = new QLabel(tr("Style Preview:"), manage_panel);
    manage_layout->addWidget(preview_label);
    
    style_preview_widget_ = new QWidget(manage_panel);
    style_preview_widget_->setMinimumHeight(100);
    style_preview_widget_->setStyleSheet("background-color: white; border: 1px solid gray;");
    manage_layout->addWidget(style_preview_widget_);
    
    // Edit style button
    edit_style_button_ = new QPushButton(tr("Edit Style Properties..."), manage_panel);
    edit_style_button_->setEnabled(false);
    manage_layout->addWidget(edit_style_button_);
    
    connect(style_preset_selector_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, [this](int index) {
                QString preset = style_preset_selector_->itemText(index);
                setCurrentStylePreset(preset);
            });
    
    connect(edit_style_button_, &QPushButton::clicked, this, &QtPropertyPanel::onEditStyleProperties);
    
    manage_layout->addStretch();
    manage_panel->setLayout(manage_layout);
    context_stack_->addWidget(manage_panel);
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
    
    bom_items_cache_ = items;
    filterBomTable();
}

void QtPropertyPanel::filterBomTable() {
    if (!bom_table_ || bom_items_cache_.isEmpty()) {
        return;
    }
    
    QString filter_text = bom_filter_ ? bom_filter_->text().toLower() : QString();
    QList<BomItem> filtered_items;
    
    for (const auto& item : bom_items_cache_) {
        if (filter_text.isEmpty() ||
            item.part_name.toLower().contains(filter_text) ||
            item.part_number.toLower().contains(filter_text) ||
            QString::number(item.quantity).contains(filter_text)) {
            filtered_items.append(item);
        }
    }
    
    bom_table_->setRowCount(filtered_items.size());
    for (int i = 0; i < filtered_items.size(); ++i) {
        const BomItem& item = filtered_items[i];
        bom_table_->setItem(i, 0, new QTableWidgetItem(item.part_name));
        bom_table_->setItem(i, 1, new QTableWidgetItem(QString::number(item.quantity)));
        bom_table_->setItem(i, 2, new QTableWidgetItem(item.part_number));
    }
    
    bom_table_->resizeColumnsToContents();
    sortBomTable();
}

void QtPropertyPanel::sortBomTable() {
    if (!bom_table_ || !bom_sort_column_) {
        return;
    }
    
    int column = bom_sort_column_->currentIndex();
    if (column >= 0 && column < bom_table_->columnCount()) {
        bom_table_->sortItems(column, Qt::AscendingOrder);
    }
}

void QtPropertyPanel::exportBomTable() {
    if (!bom_table_ || bom_table_->rowCount() == 0) {
        QMessageBox::information(this, tr("Export BOM"), tr("No BOM items to export."));
        return;
    }
    
    QString filename = QFileDialog::getSaveFileName(this, tr("Export BOM"), "", tr("CSV Files (*.csv);;All Files (*)"));
    if (filename.isEmpty()) {
        return;
    }
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Export BOM"), tr("Could not open file for writing."));
        return;
    }
    
    QTextStream out(&file);
    
    // Write header
    out << tr("Part Name") << "," << tr("Quantity") << "," << tr("Part Number") << "\n";
    
    // Write data
    for (int row = 0; row < bom_table_->rowCount(); ++row) {
        out << bom_table_->item(row, 0)->text() << ","
            << bom_table_->item(row, 1)->text() << ","
            << bom_table_->item(row, 2)->text() << "\n";
    }
    
    file.close();
    QMessageBox::information(this, tr("Export BOM"), tr("BOM exported successfully to %1").arg(filename));
}

void QtPropertyPanel::setStylePresets(const QStringList& presets) {
    if (style_preset_label_) {
        QString preset_text = tr("Available presets: ") + presets.join(", ");
        style_preset_label_->setText(preset_text);
    }
}

void QtPropertyPanel::setCurrentStylePreset(const QString& preset) {
    if (style_preset_label_) {
        style_preset_label_->setText(tr("Current: %1").arg(preset));
    }
    if (style_preset_selector_) {
        int index = style_preset_selector_->findText(preset);
        if (index >= 0) {
            style_preset_selector_->setCurrentIndex(index);
        }
    }
}

void QtPropertyPanel::setStylePresetSelector(const QStringList& presets) {
    if (style_preset_selector_) {
        style_preset_selector_->clear();
        style_preset_selector_->addItems(presets);
    }
}

void QtPropertyPanel::setLineStylesTable(const QList<QStringList>& line_styles) {
    updateLineStylesTable(line_styles);
}

void QtPropertyPanel::updateLineStylesTable(const QList<QStringList>& line_styles) {
    if (!line_styles_table_) {
        return;
    }
    
    line_styles_table_->setRowCount(line_styles.size());
    for (int i = 0; i < line_styles.size(); ++i) {
        const QStringList& row = line_styles[i];
        for (int j = 0; j < row.size() && j < line_styles_table_->columnCount(); ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(row[j]);
            if (j > 0) {  // Make non-name columns editable
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            }
            line_styles_table_->setItem(i, j, item);
        }
    }
    
    line_styles_table_->resizeColumnsToContents();
}

void QtPropertyPanel::setTextStylesTable(const QList<QStringList>& text_styles) {
    updateTextStylesTable(text_styles);
}

void QtPropertyPanel::updateTextStylesTable(const QList<QStringList>& text_styles) {
    if (!text_styles_table_) {
        return;
    }
    
    text_styles_table_->setRowCount(text_styles.size());
    for (int i = 0; i < text_styles.size(); ++i) {
        const QStringList& row = text_styles[i];
        for (int j = 0; j < row.size() && j < text_styles_table_->columnCount(); ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(row[j]);
            if (j > 0) {  // Make non-name columns editable
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            }
            text_styles_table_->setItem(i, j, item);
        }
    }
    
    text_styles_table_->resizeColumnsToContents();
}

void QtPropertyPanel::setDimensionStylesTable(const QList<QStringList>& dimension_styles) {
    updateDimensionStylesTable(dimension_styles);
}

void QtPropertyPanel::updateDimensionStylesTable(const QList<QStringList>& dimension_styles) {
    if (!dimension_styles_table_) {
        return;
    }
    
    dimension_styles_table_->setRowCount(dimension_styles.size());
    for (int i = 0; i < dimension_styles.size(); ++i) {
        const QStringList& row = dimension_styles[i];
        for (int j = 0; j < row.size() && j < dimension_styles_table_->columnCount(); ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(row[j]);
            if (j > 0) {  // Make non-name columns editable
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            }
            dimension_styles_table_->setItem(i, j, item);
        }
    }
    
    dimension_styles_table_->resizeColumnsToContents();
}

void QtPropertyPanel::setHatchStylesTable(const QList<QStringList>& hatch_styles) {
    updateHatchStylesTable(hatch_styles);
}

void QtPropertyPanel::updateHatchStylesTable(const QList<QStringList>& hatch_styles) {
    if (!hatch_styles_table_) {
        return;
    }
    
    hatch_styles_table_->setRowCount(hatch_styles.size());
    for (int i = 0; i < hatch_styles.size(); ++i) {
        const QStringList& row = hatch_styles[i];
        for (int j = 0; j < row.size() && j < hatch_styles_table_->columnCount(); ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(row[j]);
            if (j > 0) {  // Make non-name columns editable
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            }
            hatch_styles_table_->setItem(i, j, item);
        }
    }
    
    hatch_styles_table_->resizeColumnsToContents();
}

void QtPropertyPanel::setupStyleTableEditing(QTableWidget* table) {
    if (!table) {
        return;
    }
    
    connect(table, &QTableWidget::cellDoubleClicked, this, &QtPropertyPanel::onStyleTableDoubleClicked);
    connect(table, &QTableWidget::itemChanged, this, &QtPropertyPanel::onStyleTableItemChanged);
    connect(table, &QTableWidget::itemSelectionChanged, this, [this, table]() {
        bool has_selection = table->selectedItems().size() > 0;
        if (edit_style_button_) {
            edit_style_button_->setEnabled(has_selection);
        }
        
        // Determine which table was selected
        if (table == line_styles_table_) {
            current_editing_style_type_ = "Line";
        } else if (table == text_styles_table_) {
            current_editing_style_type_ = "Text";
        } else if (table == dimension_styles_table_) {
            current_editing_style_type_ = "Dimension";
        } else if (table == hatch_styles_table_) {
            current_editing_style_type_ = "Hatch";
        }
        
        if (has_selection && table->currentRow() >= 0) {
            QTableWidgetItem* name_item = table->item(table->currentRow(), 0);
            if (name_item) {
                current_editing_style_name_ = name_item->text();
            }
        }
        
        updateStylePreview();
    });
}

void QtPropertyPanel::onStyleTableDoubleClicked(int row, int column) {
    QTableWidget* table = qobject_cast<QTableWidget*>(sender());
    if (!table) {
        return;
    }
    
    QTableWidgetItem* item = table->item(row, column);
    if (item && column > 0) {  // Allow editing non-name columns
        table->editItem(item);
    }
}

void QtPropertyPanel::onStyleTableItemChanged(QTableWidgetItem* item) {
    if (!item) {
        return;
    }
    
    QTableWidget* table = qobject_cast<QTableWidget*>(sender());
    if (!table) {
        return;
    }
    
    int row = item->row();
    int col = item->column();
    
    // Get style name from first column
    QTableWidgetItem* name_item = table->item(row, 0);
    QString style_name = name_item ? name_item->text() : QString();
    
    // Determine style type
    QString style_type;
    if (table == line_styles_table_) {
        style_type = "Line";
    } else if (table == text_styles_table_) {
        style_type = "Text";
    } else if (table == dimension_styles_table_) {
        style_type = "Dimension";
    } else if (table == hatch_styles_table_) {
        style_type = "Hatch";
    }
    
    // Build properties map
    QVariantMap properties;
    QString header = table->horizontalHeaderItem(col) ? table->horizontalHeaderItem(col)->text() : QString();
    properties[header] = item->text();
    
    // Emit signal
    emit styleChanged(style_type, style_name, properties);
    
    // Update preview
    updateStylePreview();
}

void QtPropertyPanel::onEditStyleProperties() {
    QTableWidget* table = nullptr;
    if (line_styles_table_ && line_styles_table_->hasFocus()) {
        table = line_styles_table_;
    } else if (text_styles_table_ && text_styles_table_->hasFocus()) {
        table = text_styles_table_;
    } else if (dimension_styles_table_ && dimension_styles_table_->hasFocus()) {
        table = dimension_styles_table_;
    } else if (hatch_styles_table_ && hatch_styles_table_->hasFocus()) {
        table = hatch_styles_table_;
    } else if (line_styles_table_ && line_styles_table_->currentRow() >= 0) {
        table = line_styles_table_;
    } else if (text_styles_table_ && text_styles_table_->currentRow() >= 0) {
        table = text_styles_table_;
    } else if (dimension_styles_table_ && dimension_styles_table_->currentRow() >= 0) {
        table = dimension_styles_table_;
    } else if (hatch_styles_table_ && hatch_styles_table_->currentRow() >= 0) {
        table = hatch_styles_table_;
    }
    
    if (!table || table->currentRow() < 0) {
        return;
    }
    
    // Create properties dialog
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Edit Style Properties"));
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QTableWidgetItem* name_item = table->item(table->currentRow(), 0);
    QString style_name = name_item ? name_item->text() : QString();
    
    QLabel* name_label = new QLabel(tr("Style: %1").arg(style_name), &dialog);
    layout->addWidget(name_label);
    
    QFormLayout* form = new QFormLayout();
    for (int col = 0; col < table->columnCount(); ++col) {
        QTableWidgetItem* item = table->item(table->currentRow(), col);
        if (!item) {
            continue;
        }
        
        QString header = table->horizontalHeaderItem(col) ? table->horizontalHeaderItem(col)->text() : QString();
        QLineEdit* edit = new QLineEdit(item->text(), &dialog);
        if (col == 0) {
            edit->setEnabled(false);  // Name is read-only
        }
        form->addRow(header + ":", edit);
    }
    layout->addLayout(form);
    
    QPushButton* ok_button = new QPushButton(tr("OK"), &dialog);
    QPushButton* cancel_button = new QPushButton(tr("Cancel"), &dialog);
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();
    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);
    layout->addLayout(button_layout);
    
    connect(ok_button, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancel_button, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Update table items from dialog
        int form_index = 0;
        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem* item = table->item(table->currentRow(), col);
            if (!item) {
                continue;
            }
            
            QLineEdit* edit = qobject_cast<QLineEdit*>(form->itemAt(form_index, QFormLayout::FieldRole)->widget());
            if (edit && col > 0) {  // Don't update name
                item->setText(edit->text());
            }
            form_index++;
        }
        
        // Emit change signal
        QVariantMap properties;
        for (int col = 1; col < table->columnCount(); ++col) {
            QTableWidgetItem* item = table->item(table->currentRow(), col);
            if (item) {
                QString header = table->horizontalHeaderItem(col) ? table->horizontalHeaderItem(col)->text() : QString();
                properties[header] = item->text();
            }
        }
        
        emit styleChanged(current_editing_style_type_, style_name, properties);
        updateStylePreview();
    }
}

void QtPropertyPanel::onStylePreviewUpdate() {
    updateStylePreview();
}

void QtPropertyPanel::updateStylePreview() {
    if (!style_preview_widget_) {
        return;
    }
    
    // Simple preview rendering (in real implementation, would render actual style)
    QString preview_text = tr("Style Preview");
    if (!current_editing_style_name_.isEmpty()) {
        preview_text = tr("Preview: %1 (%2)").arg(current_editing_style_name_, current_editing_style_type_);
    }
    
    style_preview_widget_->setToolTip(preview_text);
}

void QtPropertyPanel::setStyleInfo(const QString& info) {
    updateStyleInfo(info);
}

void QtPropertyPanel::updateStyleInfo(const QString& info) {
    if (style_info_label_) {
        style_info_label_->setText(info);
    }
}

void QtPropertyPanel::setAnnotationItems(const QList<AnnotationItem>& items) {
    updateAnnotationTable(items);
}

void QtPropertyPanel::updateAnnotationTable(const QList<AnnotationItem>& items) {
    if (!annotation_table_) {
        return;
    }
    
    annotation_table_->setRowCount(items.size());
    for (int i = 0; i < items.size(); ++i) {
        const AnnotationItem& item = items[i];
        annotation_table_->setItem(i, 0, new QTableWidgetItem(item.text));
        annotation_table_->setItem(i, 1, new QTableWidgetItem(item.type));
        annotation_table_->setItem(i, 2, new QTableWidgetItem(QString::number(item.x, 'f', 2)));
        annotation_table_->setItem(i, 3, new QTableWidgetItem(QString::number(item.y, 'f', 2)));
        QString leader_text = item.has_leader ? tr("Yes") : tr("No");
        if (item.has_attachment && !item.attachment_entity.isEmpty()) {
            leader_text += " (" + item.attachment_entity + ")";
        }
        annotation_table_->setItem(i, 4, new QTableWidgetItem(leader_text));
    }
    
    annotation_table_->resizeColumnsToContents();
}

}  // namespace ui
}  // namespace cad
