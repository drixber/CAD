#include "QtCommunityPanel.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace cad {
namespace ui {

QtCommunityPanel::QtCommunityPanel(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void QtCommunityPanel::setupUI() {
    QVBoxLayout* main = new QVBoxLayout(this);
    main->setContentsMargins(8, 8, 8, 8);
    main->setSpacing(8);

    QLabel* title = new QLabel(tr("Community"), this);
    title->setStyleSheet("font-weight: 700; font-size: 14px; color: #212121;");
    main->addWidget(title);

    QLineEdit* search = new QLineEdit(this);
    search->setPlaceholderText(tr("Search designs..."));
    search->setClearButtonEnabled(true);
    search->setObjectName("communitySearch");
    main->addWidget(search);
    search_edit_ = search;

    QComboBox* filter = new QComboBox(this);
    filter->addItem(tr("Trending"));
    filter->addItem(tr("New"));
    filter->addItem(tr("Popular"));
    filter->addItem(tr("My uploads"));
    filter->setObjectName("communityFilter");
    main->addWidget(filter);
    filter_combo_ = filter;

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    feed_container_ = new QWidget(this);
    feed_layout_ = new QVBoxLayout(feed_container_);
    feed_layout_->setContentsMargins(0, 0, 0, 0);
    feed_layout_->setSpacing(12);
    feed_container_->setLayout(feed_layout_);

    scroll->setWidget(feed_container_);
    main->addWidget(scroll, 1);
    scroll_area_ = scroll;

    QPushButton* upload_btn = new QPushButton(tr("Share design"), this);
    upload_btn->setObjectName("communityUpload");
    main->addWidget(upload_btn);
    connect(upload_btn, &QPushButton::clicked, this, &QtCommunityPanel::uploadRequested);

    connect(search, &QLineEdit::textChanged, this, &QtCommunityPanel::onSearchChanged);
    connect(filter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtCommunityPanel::onFilterChanged);

    // Mock feed for demo
    QList<CommunityFeedItem> mock;
    mock.append({"1", tr("Bracket L-Type"), "maker1", 42, 128, tr("Simple L-bracket for mounting."), ""});
    mock.append({"2", tr("Phone Stand"), "maker2", 89, 256, tr("Adjustable phone stand with cable slot."), ""});
    mock.append({"3", tr("Gear 20 teeth"), "maker3", 15, 64, tr("Spur gear 20 teeth, 2 mm module."), ""});
    setFeedItems(mock);
}

void QtCommunityPanel::buildFeedCards() {
    while (QLayoutItem* item = feed_layout_->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    if (feed_items_.isEmpty()) {
        QLabel* empty = new QLabel(tr("No designs in feed. Connect to community API or share your first design."), feed_container_);
        empty->setWordWrap(true);
        empty->setStyleSheet("color: #757575; font-size: 12px; padding: 12px;");
        feed_layout_->addWidget(empty);
        feed_layout_->addStretch();
        return;
    }
    for (const auto& item : feed_items_) {
        QFrame* card = new QFrame(feed_container_);
        card->setObjectName("communityCard");
        card->setStyleSheet(
            "QFrame#communityCard { background: #FFFFFF; border: 1px solid #E0E0E0; border-radius: 8px; padding: 8px; }"
            "QFrame#communityCard:hover { background: #F5F5F5; border-color: #2B78E4; }"
        );
        card->setCursor(Qt::PointingHandCursor);
        card->setMinimumHeight(100);

        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(8, 8, 8, 8);
        QLabel* thumb = new QLabel(tr("3D"), card);
        thumb->setFixedHeight(60);
        thumb->setAlignment(Qt::AlignCenter);
        thumb->setStyleSheet("background: #E8E8E8; border-radius: 4px; font-size: 18px; color: #757575;");
        cardLayout->addWidget(thumb);
        QLabel* titleLabel = new QLabel(item.title, card);
        titleLabel->setStyleSheet("font-weight: 600; font-size: 12px; color: #212121;");
        titleLabel->setWordWrap(true);
        cardLayout->addWidget(titleLabel);
        QLabel* authorLabel = new QLabel(tr("by %1").arg(item.author), card);
        authorLabel->setStyleSheet("font-size: 11px; color: #757575;");
        cardLayout->addWidget(authorLabel);
        QHBoxLayout* stats = new QHBoxLayout();
        stats->addWidget(new QLabel(tr("♥ %1").arg(item.likes), card));
        stats->addWidget(new QLabel(tr("↓ %1").arg(item.downloads), card));
        stats->addStretch();
        cardLayout->addLayout(stats);

        CommunityFeedItem copy = item;
        QPushButton* viewBtn = new QPushButton(tr("View"), card);
        viewBtn->setStyleSheet("font-size: 11px; padding: 4px 8px;");
        connect(viewBtn, &QPushButton::clicked, this, [this, copy]() { showItemDetail(copy); });
        cardLayout->addWidget(viewBtn);

        feed_layout_->addWidget(card);
    }
    feed_layout_->addStretch();
}

void QtCommunityPanel::showItemDetail(const CommunityFeedItem& item) {
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle(item.title);
    QVBoxLayout* layout = new QVBoxLayout(dlg);
    layout->addWidget(new QLabel(tr("by %1").arg(item.author), dlg));
    layout->addWidget(new QLabel(tr("♥ %1  ·  ↓ %2").arg(item.likes).arg(item.downloads), dlg));
    QLabel* desc = new QLabel(item.description, dlg);
    desc->setWordWrap(true);
    layout->addWidget(desc);
    QDialogButtonBox* box = new QDialogButtonBox(dlg);
    QPushButton* downloadBtn = box->addButton(tr("Download & open"), QDialogButtonBox::AcceptRole);
    box->addButton(tr("Close"), QDialogButtonBox::RejectRole);
    connect(downloadBtn, &QPushButton::clicked, this, [this, item, dlg]() {
        emit downloadRequested(item.id);
        dlg->accept();
    });
    connect(box, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    layout->addWidget(box);
    dlg->exec();
    dlg->deleteLater();
}

void QtCommunityPanel::setFeedItems(const QList<CommunityFeedItem>& items) {
    feed_items_ = items;
    buildFeedCards();
}

void QtCommunityPanel::setFeedFromJson(const QString& json) {
    QList<CommunityFeedItem> items;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull() || !doc.isObject()) return;
    QJsonArray arr = doc.object().value("items").toArray();
    for (const QJsonValue& v : arr) {
        QJsonObject o = v.toObject();
        CommunityFeedItem item;
        item.id = o.value("id").toString();
        item.title = o.value("title").toString();
        item.author = o.value("author").toString();
        item.likes = o.value("likes").toInt(0);
        item.downloads = o.value("downloads").toInt(0);
        item.description = o.value("description").toString();
        item.thumbnailUrl = o.value("thumbnail_url").toString();
        if (!item.id.isEmpty()) items.append(item);
    }
    setFeedItems(items);
}

void QtCommunityPanel::clearFeed() {
    feed_items_.clear();
    buildFeedCards();
}

void QtCommunityPanel::setFeedUrl(const QString& url) {
    feed_url_ = url;
}

QString QtCommunityPanel::feedUrl() const {
    return feed_url_;
}

void QtCommunityPanel::onSearchChanged() {
    (void)search_edit_->text();
}

void QtCommunityPanel::onFilterChanged(int index) {
    (void)index;
}

}  // namespace ui
}  // namespace cad
