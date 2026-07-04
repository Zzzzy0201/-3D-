#include "detourdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QMap>

DetourDialog::DetourDialog(const std::vector<DetourRecommendation>& recs,
                           bool showCanteenInfo,
                           QWidget *parent)
    : QDialog(parent), currentRecs(recs), selectedId(-1)
{
    setWindowTitle("沿途景点推荐");
    resize(550, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // 列表
    listWidget = new QListWidget;
    mainLayout->addWidget(listWidget);
    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout;
    addBtn = new QPushButton("加入选中的景点");
    cancelBtn = new QPushButton("取消");
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    // 加载食堂信息（如果需要）
    QMap<QString, QString> canteenMap;
    if (showCanteenInfo) {
        canteenMap = loadCanteenInfo();
    }

    // 填充列表
    for (const auto& rec : currentRecs) {
        QString text = QString("%1 | 偏离: %2分钟 (%3米) | 停留: %4分钟")
                           .arg(QString::fromStdString(rec.name))
                           .arg(rec.detourTime, 0, 'f', 1)
                           .arg((int)rec.detourDistance)
                           .arg(rec.stayTime);
        if (showCanteenInfo) {
            QString name = QString::fromStdString(rec.name);
            if (canteenMap.contains(name)) {
                text += " | " + canteenMap[name];
            } else {
                text += " | 暂无开放信息";
            }
        }
        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, rec.poiId);
        listWidget->addItem(item);
    }

    this->setStyleSheet(
        "QDialog {"
        "   background-color: rgba(240, 245, 250, 0.92);"
        "   border-radius: 12px;"
        "   border: 1px solid rgba(0, 0, 0, 0.08);"
        "}"
        "QLabel { color: #222; }"
        "QListWidget {"
        "   background-color: rgba(255, 255, 255, 0.6);"
        "   color: #222;"
        "   border-radius: 6px;"
        "   border: 1px solid rgba(0, 0, 0, 0.08);"
        "   alternate-background-color: rgba(240, 240, 240, 0.5);"
        "}"
        "QListWidget::item { padding: 6px; }"
        "QListWidget::item:selected { background-color: rgba(41, 128, 185, 0.2); }"
        "QPushButton {"
        "   background-color: rgba(41, 128, 185, 0.5);"
        "   color: white;"
        "   border-radius: 6px;"
        "   border: none;"
        "   padding: 6px 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: rgba(41, 128, 185, 0.75); }"
        "QPushButton:pressed { background-color: rgba(41, 128, 185, 0.3); }"
        );

    connect(addBtn, &QPushButton::clicked, this, &DetourDialog::onAdd);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(listWidget, &QListWidget::itemDoubleClicked, this, &DetourDialog::onItemDoubleClicked);
}

void DetourDialog::onItemDoubleClicked(QListWidgetItem *item)
{
    selectedId = item->data(Qt::UserRole).toInt();
    accept();
}

void DetourDialog::onAdd()
{
    QListWidgetItem *cur = listWidget->currentItem();
    if (!cur) {
        QMessageBox::information(this, "提示", "请先选中一个景点");
        return;
    }
    selectedId = cur->data(Qt::UserRole).toInt();
    accept();
}

QMap<QString, QString> DetourDialog::loadCanteenInfo()
{
    QMap<QString, QString> infoMap;
    QFile file(":/data/canteen_info.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开 canteen_info.txt";
        return infoMap;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        int left = line.indexOf('(');
        if (left == -1) continue;
        QString name = line.left(left).trimmed();
        QString info = line.mid(left + 1);
        if (info.endsWith(')')) info.chop(1);
        infoMap[name] = info;
    }
    file.close();
    return infoMap;
}