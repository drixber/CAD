#pragma once

#include <QWidget>

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVariantMap>
#include <QWidget>

namespace cad {
namespace ui {

struct BomItem {
    QString part_name;
    int quantity{1};
    QString part_number;
};

struct AnnotationItem {
    QString text;
    QString type;
    double x{0.0};
    double y{0.0};
    bool has_leader{false};
    bool has_attachment{false};
    QString attachment_entity;
};

class QtPropertyPanel : public QWidget {
    Q_OBJECT

public:
    explicit QtPropertyPanel(QWidget* parent = nullptr);
    void setConstraintCount(int count);
    void setParameterCount(int count);
    void setParameterSummary(const QString& summary);
    void setIntegrationStatus(const QString& status);
    void setMateCount(int count);
    void setContextPlaceholder(const QString& context);
    void setContextCategory(const QString& category);
    void setBomItems(const QList<BomItem>& items);
    void setStylePresets(const QStringList& presets);
    void setCurrentStylePreset(const QString& preset);
    void setStyleInfo(const QString& info);
    void setAnnotationItems(const QList<AnnotationItem>& items);
    void setStylePresetSelector(const QStringList& presets);
    void setLineStylesTable(const QList<QStringList>& line_styles);
    void setTextStylesTable(const QList<QStringList>& text_styles);
    void setDimensionStylesTable(const QList<QStringList>& dimension_styles);
    void setHatchStylesTable(const QList<QStringList>& hatch_styles);
    
signals:
    void styleChanged(const QString& style_type, const QString& style_name, const QVariantMap& properties);

private slots:
    void onStyleTableDoubleClicked(int row, int column);
    void onStyleTableItemChanged(QTableWidgetItem* item);
    void onEditStyleProperties();
    void onStylePreviewUpdate();

private:
    QLabel* constraints_label_{nullptr};
    QLabel* parameters_label_{nullptr};
    QLabel* parameters_summary_{nullptr};
    QLabel* integration_status_{nullptr};
    QLabel* mates_label_{nullptr};
    QLabel* context_label_{nullptr};
    QStackedWidget* context_stack_{nullptr};
    QTableWidget* bom_table_{nullptr};
    QTableWidget* annotation_table_{nullptr};
    QLabel* style_preset_label_{nullptr};
    QLabel* style_info_label_{nullptr};
    QComboBox* style_preset_selector_{nullptr};
    QTableWidget* line_styles_table_{nullptr};
    QTableWidget* text_styles_table_{nullptr};
    QTableWidget* dimension_styles_table_{nullptr};
    QTableWidget* hatch_styles_table_{nullptr};
    QWidget* style_preview_widget_{nullptr};
    QPushButton* edit_style_button_{nullptr};
    QLineEdit* bom_filter_{nullptr};
    QComboBox* bom_sort_column_{nullptr};
    QPushButton* bom_export_button_{nullptr};
    QList<BomItem> bom_items_cache_;
    QString current_editing_style_type_;
    QString current_editing_style_name_;
    
    void updateBomTable(const QList<BomItem>& items);
    void updateStyleInfo(const QString& info);
    void updateAnnotationTable(const QList<AnnotationItem>& items);
    void updateLineStylesTable(const QList<QStringList>& line_styles);
    void updateTextStylesTable(const QList<QStringList>& text_styles);
    void updateDimensionStylesTable(const QList<QStringList>& dimension_styles);
    void updateHatchStylesTable(const QList<QStringList>& hatch_styles);
    void updateStylePreview();
    void setupStyleTableEditing(QTableWidget* table);
    void filterBomTable();
    void sortBomTable();
    void exportBomTable();
};

}  // namespace ui
}  // namespace cad
