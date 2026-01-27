#pragma once

#include <QWidget>

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPointF>
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
    QList<QPointF> leader_points;
    QPointF attachment_point;
};

class QtPropertyPanel : public ::QWidget {
    Q_OBJECT

public:
    explicit QtPropertyPanel(::QWidget* parent = nullptr);
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
    void annotationChanged(const QString& annotation_id, const QVariantMap& properties);
    void annotationPositionChanged(const QString& annotation_id, double x, double y);
    void annotationLeaderChanged(const QString& annotation_id, const QList<QPointF>& leader_points);

private slots:
    void onStyleTableDoubleClicked(int row, int column);
    void onStyleTableItemChanged(QTableWidgetItem* item);
    void onEditStyleProperties();
    void onStylePreviewUpdate();
    void onAnnotationTableDoubleClicked(int row, int column);
    void onAnnotationTableItemChanged(QTableWidgetItem* item);
    void onEditAnnotationLeader();
    void onAnnotationPositionDrag();

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
    QPushButton* edit_leader_button_{nullptr};
    QPushButton* drag_annotation_button_{nullptr};
    QLabel* style_preset_label_{nullptr};
    QLabel* style_info_label_{nullptr};
    QComboBox* style_preset_selector_{nullptr};
    QTableWidget* line_styles_table_{nullptr};
    QTableWidget* text_styles_table_{nullptr};
    QTableWidget* dimension_styles_table_{nullptr};
    QTableWidget* hatch_styles_table_{nullptr};
    QLabel* style_preview_widget_{nullptr};
    QPushButton* edit_style_button_{nullptr};
    QLineEdit* bom_filter_{nullptr};
    QComboBox* bom_sort_column_{nullptr};
    QComboBox* bom_filter_column_{nullptr};
    QComboBox* bom_filter_operator_{nullptr};
    QLineEdit* bom_filter_value_{nullptr};
    QPushButton* bom_sort_ascending_{nullptr};
    QPushButton* bom_sort_descending_{nullptr};
    QPushButton* bom_export_button_{nullptr};
    QPushButton* bom_clear_filters_{nullptr};
    QComboBox* bom_filter_preset_{nullptr};
    QPushButton* bom_save_preset_button_{nullptr};
    QList<BomItem> bom_items_cache_;
    QList<int> bom_sort_columns_;
    Qt::SortOrder bom_sort_order_{Qt::AscendingOrder};
    QList<AnnotationItem> annotation_items_cache_;
    QString current_editing_style_type_;
    QString current_editing_style_name_;
    int current_editing_annotation_row_{-1};
    
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
    void applyBomFilter();
    void sortBomTable();
    void exportBomTable();
    void applyMultiColumnSort();
    void addSortColumn(int column);
    void removeSortColumn(int column);
    void clearAllFilters();
    bool matchesFilter(const BomItem& item, const QString& filter_text, int filter_column) const;
    void saveFilterPreset(const QString& preset_name);
    void loadFilterPreset(const QString& preset_name);
    QStringList getFilterPresets() const;
    void exportBomToFormat(const QString& format, const QString& filename);
    void setupAnnotationTableEditing();
    void editAnnotationLeader(int row);
    QString getAnnotationId(int row) const;
    void updateAnnotationPreview();
};

}  // namespace ui
}  // namespace cad
