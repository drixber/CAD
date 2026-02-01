#pragma once

#include <QWidget>
#include <QStringList>
#include <QList>

class QLineEdit;
class QComboBox;
class QScrollArea;
class QVBoxLayout;

namespace cad {
namespace ui {

struct CommunityFeedItem {
    QString id;
    QString title;
    QString author;
    int likes{0};
    int downloads{0};
    QString description;
    QString thumbnailUrl;  // optional; empty = placeholder
};

class QtCommunityPanel : public QWidget {
    Q_OBJECT

public:
    explicit QtCommunityPanel(QWidget* parent = nullptr);

    void setFeedUrl(const QString& url);
    QString feedUrl() const;
    void setFeedItems(const QList<CommunityFeedItem>& items);
    /** Parse JSON from API (items array with id, title, author, likes, downloads, description) and refresh feed. */
    void setFeedFromJson(const QString& json);
    void clearFeed();

signals:
    void itemSelected(const QString& id);
    void downloadRequested(const QString& id);
    void uploadRequested();

private:
    void setupUI();
    void buildFeedCards();
    void onSearchChanged();
    void onFilterChanged(int index);
    void showItemDetail(const CommunityFeedItem& item);

    QLineEdit* search_edit_{nullptr};
    QComboBox* filter_combo_{nullptr};
    QScrollArea* scroll_area_{nullptr};
    QVBoxLayout* feed_layout_{nullptr};
    QWidget* feed_container_{nullptr};
    QString feed_url_;
    QList<CommunityFeedItem> feed_items_;
};

}  // namespace ui
}  // namespace cad
