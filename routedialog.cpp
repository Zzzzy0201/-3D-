#include "routedialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

RouteDialog::RouteDialog(const QString &duration, const QStringList &sceneryList, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("导航路径最终确认");
    setMinimumSize(360, 280);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *timeLabel = new QLabel("预计出行时间: " + duration, this);
    timeLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #800000; margin: 5px 0;");
    layout->addWidget(timeLabel);
    layout->addWidget(new QLabel("导航规划途径主要地标:", this));
    QListWidget *waypointList = new QListWidget(this);
    waypointList->addItems(sceneryList);
    layout->addWidget(waypointList);
    QPushButton *confirmBtn = new QPushButton("开启3D实景导航", this);
    confirmBtn->setStyleSheet("background: #800000; color: white; padding: 10px; font-weight: bold; border-radius: 4px; font-size: 14px;");
    connect(confirmBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(confirmBtn);
}