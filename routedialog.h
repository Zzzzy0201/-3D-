#ifndef ROUTEDIALOG_H
#define ROUTEDIALOG_H

#include <QDialog>
#include <QStringList>

class RouteDialog : public QDialog
{
    Q_OBJECT
public:
    RouteDialog(const QString &duration, const QStringList &sceneryList, QWidget *parent = nullptr);
};

#endif // ROUTEDIALOG_H
