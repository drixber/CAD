#include "QtAISettingsDialog.h"
#include <QMessageBox>

namespace cad {
namespace ui {

QtAISettingsDialog::QtAISettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("AI Settings"));
    setModal(true);
    setMinimumWidth(500);
    setupUI();
}

void QtAISettingsDialog::setupUI() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(16);
    main_layout->setContentsMargins(16, 16, 16, 16);
    
    // Provider Selection
    QGroupBox* provider_group = new QGroupBox(tr("AI Provider"), this);
    QFormLayout* provider_layout = new QFormLayout(provider_group);
    
    provider_combo_ = new QComboBox(this);
    provider_combo_->addItem(tr("OpenAI"), "openai");
    provider_combo_->addItem(tr("Anthropic"), "anthropic");
    provider_combo_->addItem(tr("Grok"), "grok");
    provider_combo_->setCurrentIndex(0);
    provider_layout->addRow(tr("Provider:"), provider_combo_);
    
    model_combo_ = new QComboBox(this);
    model_combo_->addItem("gpt-4", "gpt-4");
    model_combo_->addItem("gpt-4-turbo-preview", "gpt-4-turbo-preview");
    model_combo_->addItem("gpt-3.5-turbo", "gpt-3.5-turbo");
    provider_layout->addRow(tr("Model:"), model_combo_);
    
    main_layout->addWidget(provider_group);
    
    // API Keys
    QGroupBox* keys_group = new QGroupBox(tr("API Keys"), this);
    QFormLayout* keys_layout = new QFormLayout(keys_group);
    
    openai_key_edit_ = new QLineEdit(this);
    openai_key_edit_->setPlaceholderText(tr("sk-..."));
    openai_key_edit_->setEchoMode(QLineEdit::Password);
    keys_layout->addRow(tr("OpenAI Key:"), openai_key_edit_);
    
    anthropic_key_edit_ = new QLineEdit(this);
    anthropic_key_edit_->setPlaceholderText(tr("sk-ant-..."));
    anthropic_key_edit_->setEchoMode(QLineEdit::Password);
    anthropic_key_edit_->hide();
    keys_layout->addRow(tr("Anthropic Key:"), anthropic_key_edit_);
    
    grok_key_edit_ = new QLineEdit(this);
    grok_key_edit_->setPlaceholderText(tr("xai-..."));
    grok_key_edit_->setEchoMode(QLineEdit::Password);
    grok_key_edit_->hide();
    keys_layout->addRow(tr("Grok Key:"), grok_key_edit_);
    
    main_layout->addWidget(keys_group);
    
    // Advanced Settings
    QGroupBox* advanced_group = new QGroupBox(tr("Advanced Settings"), this);
    QFormLayout* advanced_layout = new QFormLayout(advanced_group);
    
    temperature_spin_ = new QDoubleSpinBox(this);
    temperature_spin_->setRange(0.0, 2.0);
    temperature_spin_->setSingleStep(0.1);
    temperature_spin_->setValue(0.7);
    temperature_spin_->setDecimals(1);
    advanced_layout->addRow(tr("Temperature:"), temperature_spin_);
    
    max_tokens_spin_ = new QSpinBox(this);
    max_tokens_spin_->setRange(100, 8000);
    max_tokens_spin_->setSingleStep(100);
    max_tokens_spin_->setValue(2000);
    advanced_layout->addRow(tr("Max Tokens:"), max_tokens_spin_);
    
    streaming_check_ = new QCheckBox(tr("Enable Streaming Responses"), this);
    streaming_check_->setChecked(true);
    advanced_layout->addRow(streaming_check_);
    
    main_layout->addWidget(advanced_group);
    
    // Test Connection
    test_button_ = new QPushButton(tr("Test Connection"), this);
    test_button_->setStyleSheet(
        "QPushButton {"
        "    background: #2B78E4;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 8px 16px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: #1E5AA8;"
        "}"
    );
    main_layout->addWidget(test_button_);
    
    test_result_label_ = new QLabel(this);
    test_result_label_->setWordWrap(true);
    test_result_label_->hide();
    main_layout->addWidget(test_result_label_);
    
    main_layout->addStretch();
    
    // Buttons
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();
    
    cancel_button_ = new QPushButton(tr("Cancel"), this);
    cancel_button_->setStyleSheet(
        "QPushButton {"
        "    background: transparent;"
        "    color: #757575;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    padding: 8px 16px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: #F5F5F5;"
        "}"
    );
    button_layout->addWidget(cancel_button_);
    
    save_button_ = new QPushButton(tr("Save"), this);
    save_button_->setStyleSheet(
        "QPushButton {"
        "    background: #2B78E4;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 8px 16px;"
        "    font-weight: 600;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: #1E5AA8;"
        "}"
    );
    button_layout->addWidget(save_button_);
    
    main_layout->addLayout(button_layout);
    
    // Connections
    connect(provider_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { onProviderChanged(provider_combo_->currentData().toString()); });
    connect(test_button_, &QPushButton::clicked, this, &QtAISettingsDialog::onTestClicked);
    connect(save_button_, &QPushButton::clicked, this, &QtAISettingsDialog::onSaveClicked);
    connect(cancel_button_, &QPushButton::clicked, this, &QtAISettingsDialog::onCancelClicked);
    connect(openai_key_edit_, &QLineEdit::textChanged, this, &QtAISettingsDialog::validateInputs);
    connect(anthropic_key_edit_, &QLineEdit::textChanged, this, &QtAISettingsDialog::validateInputs);
    connect(grok_key_edit_, &QLineEdit::textChanged, this, &QtAISettingsDialog::validateInputs);
    
    updateProviderVisibility();
    validateInputs();
}

QString QtAISettingsDialog::getOpenAIKey() const {
    return openai_key_edit_->text().trimmed();
}

QString QtAISettingsDialog::getAnthropicKey() const {
    return anthropic_key_edit_->text().trimmed();
}

QString QtAISettingsDialog::getGrokKey() const {
    return grok_key_edit_->text().trimmed();
}

QString QtAISettingsDialog::getSelectedProvider() const {
    return provider_combo_->currentData().toString();
}

QString QtAISettingsDialog::getSelectedModel() const {
    return model_combo_->currentData().toString();
}

double QtAISettingsDialog::getTemperature() const {
    return temperature_spin_->value();
}

int QtAISettingsDialog::getMaxTokens() const {
    return max_tokens_spin_->value();
}

bool QtAISettingsDialog::getStreamingEnabled() const {
    return streaming_check_->isChecked();
}

void QtAISettingsDialog::setOpenAIKey(const QString& key) {
    openai_key_edit_->setText(key);
}

void QtAISettingsDialog::setAnthropicKey(const QString& key) {
    anthropic_key_edit_->setText(key);
}

void QtAISettingsDialog::setGrokKey(const QString& key) {
    grok_key_edit_->setText(key);
}

void QtAISettingsDialog::setSelectedProvider(const QString& provider) {
    int index = provider_combo_->findData(provider);
    if (index >= 0) {
        provider_combo_->setCurrentIndex(index);
        onProviderChanged(provider);
    }
}

void QtAISettingsDialog::setSelectedModel(const QString& model) {
    int index = model_combo_->findData(model);
    if (index >= 0) {
        model_combo_->setCurrentIndex(index);
    }
}

void QtAISettingsDialog::setTemperature(double temp) {
    temperature_spin_->setValue(temp);
}

void QtAISettingsDialog::setMaxTokens(int tokens) {
    max_tokens_spin_->setValue(tokens);
}

void QtAISettingsDialog::setStreamingEnabled(bool enabled) {
    streaming_check_->setChecked(enabled);
}

void QtAISettingsDialog::setTestResult(const QString& result, bool success) {
    test_result_label_->setText(result);
    test_result_label_->setStyleSheet(
        success ? "color: #4CAF50; font-size: 12px;"
                : "color: #F44336; font-size: 12px;"
    );
    test_result_label_->show();
}

void QtAISettingsDialog::onProviderChanged(const QString& provider) {
    updateModelList(provider);
    updateProviderVisibility();
    validateInputs();
}

void QtAISettingsDialog::onTestClicked() {
    QString provider = getSelectedProvider();
    emit testConnectionRequested(provider);
}

void QtAISettingsDialog::onSaveClicked() {
    emit saveRequested();
    accept();
}

void QtAISettingsDialog::onCancelClicked() {
    emit cancelRequested();
    reject();
}

void QtAISettingsDialog::validateInputs() {
    QString provider = getSelectedProvider();
    bool has_key = false;
    
    if (provider == "openai") {
        has_key = !getOpenAIKey().isEmpty();
    } else if (provider == "anthropic") {
        has_key = !getAnthropicKey().isEmpty();
    } else if (provider == "grok") {
        has_key = !getGrokKey().isEmpty();
    }
    
    save_button_->setEnabled(has_key);
    test_button_->setEnabled(has_key);
}

void QtAISettingsDialog::updateModelList(const QString& provider) {
    model_combo_->clear();
    
    if (provider == "openai") {
        model_combo_->addItem("gpt-4", "gpt-4");
        model_combo_->addItem("gpt-4-turbo-preview", "gpt-4-turbo-preview");
        model_combo_->addItem("gpt-4-0125-preview", "gpt-4-0125-preview");
        model_combo_->addItem("gpt-3.5-turbo", "gpt-3.5-turbo");
        model_combo_->addItem("gpt-3.5-turbo-16k", "gpt-3.5-turbo-16k");
    } else if (provider == "anthropic") {
        model_combo_->addItem("claude-3-5-sonnet-20241022", "claude-3-5-sonnet-20241022");
        model_combo_->addItem("claude-3-5-haiku-20241022", "claude-3-5-haiku-20241022");
        model_combo_->addItem("claude-3-opus-20240229", "claude-3-opus-20240229");
        model_combo_->addItem("claude-3-sonnet-20240229", "claude-3-sonnet-20240229");
        model_combo_->addItem("claude-3-haiku-20240307", "claude-3-haiku-20240307");
    } else if (provider == "grok") {
        model_combo_->addItem("grok-2-latest", "grok-2-latest");
        model_combo_->addItem("grok-2-mini", "grok-2-mini");
    }
}

void QtAISettingsDialog::updateProviderVisibility() {
    QString provider = getSelectedProvider();
    
    if (provider == "openai") {
        openai_key_edit_->show();
        anthropic_key_edit_->hide();
        grok_key_edit_->hide();
    } else if (provider == "anthropic") {
        openai_key_edit_->hide();
        anthropic_key_edit_->show();
        grok_key_edit_->hide();
    } else if (provider == "grok") {
        openai_key_edit_->hide();
        anthropic_key_edit_->hide();
        grok_key_edit_->show();
    }
}

}  // namespace ui
}  // namespace cad
