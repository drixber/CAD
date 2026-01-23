#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>

namespace cad {
namespace ui {

class QtAIChatPanel : public QWidget {
    Q_OBJECT

public:
    explicit QtAIChatPanel(QWidget* parent = nullptr);
    ~QtAIChatPanel() override = default;
    
    // Message management
    void addUserMessage(const QString& message);
    void addAIMessage(const QString& message);
    void appendToAIMessage(const QString& chunk); // For streaming
    void clearMessages();
    
    // State management
    void setThinking(bool thinking);
    bool isThinking() const { return thinking_; }
    
    // Settings
    void setModelName(const QString& model);
    QString getModelName() const;

signals:
    void messageSent(const QString& message);
    void clearRequested();
    void settingsRequested();

private slots:
    void onSendClicked();
    void onClearClicked();
    void onSettingsClicked();
    void onInputReturnPressed();
    void onInputChanged();

private:
    QTextEdit* chat_display_{nullptr};
    QLineEdit* input_field_{nullptr};
    QPushButton* send_button_{nullptr};
    QPushButton* clear_button_{nullptr};
    QPushButton* settings_button_{nullptr};
    QLabel* model_label_{nullptr};
    QLabel* thinking_label_{nullptr};
    
    bool thinking_{false};
    QString current_ai_message_; // For streaming
    
    void setupUI();
    void formatMessage(const QString& role, const QString& content);
    QString formatMarkdown(const QString& text) const;
    void scrollToBottom();
};

}  // namespace ui
}  // namespace cad
