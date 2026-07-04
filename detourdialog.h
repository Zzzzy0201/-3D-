#ifndef DETOURDIALOG_H
#define DETOURDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include "routeplanner.h"

class DetourDialog : public QDialog
{
    Q_OBJECT
public:
    DetourDialog(const std::vector<DetourRecommendation>& recs,bool showCanteenInfo, QWidget *parent = nullptr);
    int getSelectedPoiId() const { return selectedId; }

private slots:
    void onItemDoubleClicked(QListWidgetItem* item);
    void onAdd();

private:
    QListWidget *listWidget;
    QPushButton *addBtn;
    QPushButton *cancelBtn;
    std::vector<DetourRecommendation> currentRecs;
    int selectedId;
    QMap<QString, QString> loadCanteenInfo();
};

#endif