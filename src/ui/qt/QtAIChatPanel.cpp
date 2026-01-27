#include "QtAIChatPanel.h"
#include <QScrollBar>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QDateTime>
#include <QRegularExpression>

namespace cad {
namespace ui {

QtAIChatPanel::QtAIChatPanel(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void QtAIChatPanel::setupUI() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(8, 8, 8, 8);
    main_layout->setSpacing(8);
    
    // Header with model info
    QHBoxLayout* header_layout = new QHBoxLayout();
    model_label_ = new QLabel(tr("Model: Not configured"), this);
    model_label_->setStyleSheet("font-weight: 600; color: #757575; font-size: 11px;");
    header_layout->addWidget(model_label_);
    
    header_layout->addStretch();
    
    thinking_label_ = new QLabel(tr("Thinking..."), this);
    thinking_label_->setStyleSheet("color: #2B78E4; font-size: 11px;");
    thinking_label_->hide();
    header_layout->addWidget(thinking_label_);
    
    main_layout->addLayout(header_layout);
    
    // Chat display
    chat_display_ = new QTextEdit(this);
    chat_display_->setReadOnly(true);
    chat_display_->setStyleSheet(
        "QTextEdit {"
        "    background: #FFFFFF;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    font-size: 13px;"
        "    line-height: 1.5;"
        "}"
    );
    main_layout->addWidget(chat_display_, 1);
    
    // Input area
    QHBoxLayout* input_layout = new QHBoxLayout();
    input_layout->setSpacing(8);
    
    input_field_ = new QLineEdit(this);
    input_field_->setPlaceholderText(tr("Ask AI about CAD operations..."));
    input_field_->setStyleSheet(
        "QLineEdit {"
        "    padding: 8px 12px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #2B78E4;"
        "    padding: 7px 11px;"
        "}"
    );
    input_layout->addWidget(input_field_, 1);
    
    send_button_ = new QPushButton(tr("Send"), this);
    send_button_->setStyleSheet(
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
        "QPushButton:pressed {"
        "    background: #164080;"
        "}"
        "QPushButton:disabled {"
        "    background: #BDBDBD;"
        "}"
    );
    send_button_->setEnabled(false);
    input_layout->addWidget(send_button_);
    
    main_layout->addLayout(input_layout);
    
    // Button row
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();
    
    clear_button_ = new QPushButton(tr("Clear"), this);
    clear_button_->setStyleSheet(
        "QPushButton {"
        "    background: transparent;"
        "    color: #757575;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    padding: 6px 12px;"
        "    font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "    background: #F5F5F5;"
        "    border-color: #B0B0B0;"
        "}"
    );
    button_layout->addWidget(clear_button_);
    
    settings_button_ = new QPushButton(tr("Settings"), this);
    settings_button_->setStyleSheet(clear_button_->styleSheet());
    button_layout->addWidget(settings_button_);
    
    main_layout->addLayout(button_layout);
    
    // Connections
    connect(send_button_, &QPushButton::clicked, this, &QtAIChatPanel::onSendClicked);
    connect(clear_button_, &QPushButton::clicked, this, &QtAIChatPanel::onClearClicked);
    connect(settings_button_, &QPushButton::clicked, this, &QtAIChatPanel::onSettingsClicked);
    connect(input_field_, &QLineEdit::returnPressed, this, &QtAIChatPanel::onInputReturnPressed);
    connect(input_field_, &QLineEdit::textChanged, this, &QtAIChatPanel::onInputChanged);
}

void QtAIChatPanel::addUserMessage(const QString& message) {
    formatMessage("user", message);
    scrollToBottom();
}

void QtAIChatPanel::addAIMessage(const QString& message) {
    current_ai_message_ = message;
    formatMessage("assistant", message);
    scrollToBottom();
}

void QtAIChatPanel::appendToAIMessage(const QString& chunk) {
    current_ai_message_ += chunk;
    
    // Update the last AI message
    QTextCursor cursor = chat_display_->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    // Find the last AI message block and update it
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    
    // Simple approach: append to end
    chat_display_->moveCursor(QTextCursor::End);
    chat_display_->insertPlainText(chunk);
    scrollToBottom();
}

void QtAIChatPanel::clearMessages() {
    chat_display_->clear();
    current_ai_message_.clear();
}

void QtAIChatPanel::setThinking(bool thinking) {
    thinking_ = thinking;
    if (thinking) {
        thinking_label_->show();
        send_button_->setEnabled(false);
        input_field_->setEnabled(false);
    } else {
        thinking_label_->hide();
        send_button_->setEnabled(!input_field_->text().trimmed().isEmpty());
        input_field_->setEnabled(true);
    }
}

void QtAIChatPanel::setModelName(const QString& model) {
    model_label_->setText(tr("Model: %1").arg(model.isEmpty() ? tr("Not configured") : model));
}

QString QtAIChatPanel::getModelName() const {
    QString text = model_label_->text();
    if (text.startsWith("Model: ")) {
        return text.mid(7);
    }
    return "";
}

void QtAIChatPanel::onSendClicked() {
    QString message = input_field_->text().trimmed();
    if (!message.isEmpty()) {
        addUserMessage(message);
        emit messageSent(message);
        input_field_->clear();
        setThinking(true);
    }
}

void QtAIChatPanel::onClearClicked() {
    clearMessages();
    emit clearRequested();
}

void QtAIChatPanel::onSettingsClicked() {
    emit settingsRequested();
}

void QtAIChatPanel::onInputReturnPressed() {
    onSendClicked();
}

void QtAIChatPanel::onInputChanged() {
    bool has_text = !input_field_->text().trimmed().isEmpty();
    send_button_->setEnabled(has_text && !thinking_);
}

void QtAIChatPanel::formatMessage(const QString& role, const QString& content) {
    QTextCursor cursor = chat_display_->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    // Format based on role
    QTextCharFormat format;
    if (role == "user") {
        format.setForeground(QColor("#212121"));
        format.setFontWeight(QFont::Bold);
        cursor.insertText(tr("You: "), format);
        
        format.setFontWeight(QFont::Normal);
        cursor.insertText(content + "\n\n", format);
    } else if (role == "assistant") {
        format.setForeground(QColor("#2B78E4"));
        format.setFontWeight(QFont::Bold);
        cursor.insertText(tr("AI: "), format);
        
        format.setForeground(QColor("#212121"));
        format.setFontWeight(QFont::Normal);
        
        // Simple markdown formatting
        QString formatted = formatMarkdown(content);
        cursor.insertText(formatted + "\n\n", format);
    }
    
    scrollToBottom();
}

QString QtAIChatPanel::formatMarkdown(const QString& text) const {
    // Simple markdown formatting
    QString result = text;
    
    // Bold: **text**
    result.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
    
    // Code: `code`
    result.replace(QRegularExpression("`(.+?)`"), "<code style='background: #F5F5F5; padding: 2px 4px; border-radius: 3px;'>\\1</code>");
    
    // Line breaks
    result.replace("\n", "<br>");
    
    return result;
}

void QtAIChatPanel::scrollToBottom() {
    QScrollBar* scrollbar = chat_display_->verticalScrollBar();
    if (scrollbar) {
        scrollbar->setValue(scrollbar->maximum());
    }
}

}  // namespace ui
}  // namespace cad
