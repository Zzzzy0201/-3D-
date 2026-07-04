#ifndef PREFERENCE_ORDER_DIALOG_H
#define PREFERENCE_ORDER_DIALOG_H

#include <QDialog>
#include <QStringList>
#include <QTime>
#include <QMap>

class QListWidget;
class QListWidgetItem;
class QComboBox;
class QTimeEdit;

class PreferenceOrderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferenceOrderDialog(const QStringList &allTags, const QStringList &gateNames, QWidget *parent = nullptr);
    QStringList getOrderedTags() const;
    int getDayOfWeek() const;
    QTime getStartTime() const;
    QString getSelectedGate() const;

private slots:
    void onItemClicked(QListWidgetItem *item);

private:
    void updateList();
    QListWidget *tagList;
    QComboBox *dayCombo;
    QTimeEdit *timeEdit;
    QStringList allTags;
    QStringList selectedTags;
    QMap<QString, int> selectedOrder;
    QComboBox *m_gateCombo;
    QStringList m_gateNames;

};


#endif // PREFERENCE_ORDER_DIALOG_H