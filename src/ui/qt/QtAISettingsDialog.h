#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

namespace cad {
namespace ui {

class QtAISettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit QtAISettingsDialog(QWidget* parent = nullptr);
    ~QtAISettingsDialog() override = default;
    
    // Getters
    QString getOpenAIKey() const;
    QString getAnthropicKey() const;
    QString getSelectedProvider() const;
    QString getSelectedModel() const;
    double getTemperature() const;
    int getMaxTokens() const;
    bool getStreamingEnabled() const;
    
    // Setters
    void setOpenAIKey(const QString& key);
    void setAnthropicKey(const QString& key);
    void setSelectedProvider(const QString& provider);
    void setSelectedModel(const QString& model);
    void setTemperature(double temp);
    void setMaxTokens(int tokens);
    void setStreamingEnabled(bool enabled);
    
    // Test connection
    void setTestResult(const QString& result, bool success);

signals:
    void testConnectionRequested(const QString& provider);
    void saveRequested();
    void cancelRequested();

private slots:
    void onProviderChanged(const QString& provider);
    void onTestClicked();
    void onSaveClicked();
    void onCancelClicked();
    void validateInputs();

private:
    QComboBox* provider_combo_{nullptr};
    QComboBox* model_combo_{nullptr};
    QLineEdit* openai_key_edit_{nullptr};
    QLineEdit* anthropic_key_edit_{nullptr};
    QDoubleSpinBox* temperature_spin_{nullptr};
    QSpinBox* max_tokens_spin_{nullptr};
    QCheckBox* streaming_check_{nullptr};
    QPushButton* test_button_{nullptr};
    QLabel* test_result_label_{nullptr};
    QPushButton* save_button_{nullptr};
    QPushButton* cancel_button_{nullptr};
    
    void setupUI();
    void updateModelList(const QString& provider);
    void updateProviderVisibility();
};

}  // namespace ui
}  // namespace cad
