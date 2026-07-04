#include "preference_order_dialog.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>
#include <QTimeEdit>
#include <QDialogButtonBox>
#include <QLabel>

PreferenceOrderDialog::PreferenceOrderDialog(const QStringList &allTags, const QStringList &gateNames, QWidget *parent)
    : QDialog(parent), allTags(allTags), m_gateNames(gateNames)
{
    setWindowTitle("偏好选择");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *tagGroup = new QGroupBox("请点击标签选择偏好（多选，按优先级排序；可不选）", this);
    tagList = new QListWidget;
    tagList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(tagList, &QListWidget::itemClicked, this, &PreferenceOrderDialog::onItemClicked);
    QVBoxLayout *tagLayout = new QVBoxLayout;
    tagLayout->addWidget(tagList);
    tagGroup->setLayout(tagLayout);
    mainLayout->addWidget(tagGroup);

    // 起点选择
    QGroupBox *gateGroup = new QGroupBox("选择起点入口", this);
    QHBoxLayout *gateLayout = new QHBoxLayout;
    QLabel *gateLabel = new QLabel("起点：");
    m_gateCombo = new QComboBox;
    m_gateCombo->addItems(m_gateNames);
    gateLayout->addWidget(gateLabel);
    gateLayout->addWidget(m_gateCombo);
    gateLayout->addStretch();
    gateGroup->setLayout(gateLayout);
    mainLayout->addWidget(gateGroup);

    // 时间选择
    QGroupBox *timeGroup = new QGroupBox("参观时间", this);
    QFormLayout *timeLayout = new QFormLayout;
    dayCombo = new QComboBox;
    dayCombo->addItems({"周一", "周二", "周三", "周四", "周五", "周六", "周日"});
    timeEdit = new QTimeEdit;
    timeEdit->setTime(QTime(8, 0));
    timeLayout->addRow("星期：", dayCombo);
    timeLayout->addRow("开始时间：", timeEdit);
    timeGroup->setLayout(timeLayout);
    mainLayout->addWidget(timeGroup);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    mainLayout->addWidget(buttonBox);

    updateList();
    this->setStyleSheet(
        "QDialog {"
        "   background-color: rgba(240, 245, 250, 0.92);"
        "   border-radius: 12px;"
        "   border: 1px solid rgba(0, 0, 0, 0.08);"
        "}"
        "QGroupBox {"
        "   color: #222;"
        "   font-weight: bold;"
        "   border: 1px solid rgba(0, 0, 0, 0.08);"
        "   border-radius: 8px;"
        "   margin-top: 12px;"
        "   padding-top: 10px;"
        "}"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; }"
        "QLabel { color: #222; }"
        "QListWidget {"
        "   background-color: rgba(255, 255, 255, 0.6);"
        "   color: #222;"
        "   border-radius: 6px;"
        "   border: 1px solid rgba(0, 0, 0, 0.08);"
        "   alternate-background-color: rgba(240, 240, 240, 0.5);"
        "}"
        "QListWidget::item { padding: 4px; }"
        "QListWidget::item:selected { background-color: rgba(41, 128, 185, 0.2); }"
        "QComboBox, QTimeEdit {"
        "   background-color: rgba(255, 255, 255, 0.6);"
        "   color: #222;"
        "   border-radius: 6px;"
        "   border: 1px solid rgba(0, 0, 0, 0.08);"
        "   padding: 4px 6px;"
        "}"
        "QComboBox::drop-down { border: none; }"
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
}

void PreferenceOrderDialog::onItemClicked(QListWidgetItem *item)
{
    QString tag = item->data(Qt::UserRole).toString(); // 获取原始标签名
    if (selectedOrder.contains(tag)) {
        selectedOrder.remove(tag);
        selectedTags.removeAll(tag);
    } else {
        selectedTags.append(tag);
        selectedOrder[tag] = selectedTags.size();
    }
    updateList();
}

void PreferenceOrderDialog::updateList()
{
    tagList->clear();

    // 显示已选标签（按顺序带编号）
    for (int i = 0; i < selectedTags.size(); ++i) {
        const QString &tag = selectedTags[i];
        QString display = QString("%1. %2").arg(i+1).arg(tag);
        QListWidgetItem *item = new QListWidgetItem(display);
        item->setData(Qt::UserRole, tag);
        item->setBackground(QColor(200, 255, 200));
        tagList->addItem(item);
    }

    // 显示未选标签（原始顺序，排除已选）
    for (const QString &tag : allTags) {
        if (!selectedOrder.contains(tag)) {
            QListWidgetItem *item = new QListWidgetItem(tag);
            item->setData(Qt::UserRole, tag);
            item->setBackground(Qt::white);
            tagList->addItem(item);
        }
    }
}

QStringList PreferenceOrderDialog::getOrderedTags() const
{
    return selectedTags;
}

int PreferenceOrderDialog::getDayOfWeek() const
{
    return dayCombo->currentIndex();
}

QTime PreferenceOrderDialog::getStartTime() const
{
    return timeEdit->time();
}

QString PreferenceOrderDialog::getSelectedGate() const
{
    return m_gateCombo->currentText();
}