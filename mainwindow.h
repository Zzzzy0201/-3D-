#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "waypointreorderwidget.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QStringList>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCompleter>

class MapWidget;
class ThumbnailWidget;
class RoutePlanner;
class Campus3DWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    bool onBuildingClicked(const QString &name, const QPointF &);
    void onDeleteDestination(int index);
    void onSearchClicked();
    void onAddButtonClicked();
    void onCancelButtonClicked();
    void onConfirmDestinations();
    void onCompleterActivated(const QString &text);
    void switchTo2DMapView();
    void switchTo3DView();
    void switchToMenuView();
    void onDetourRecommend();
    void onPathReordered(const QVector<QPointF> &newPath);
    void onShowReorderPanel();

private:
    void updateDestinationList();
    void setupCompleter();
    void applyPreference(const QStringList &selectedTags);
    void checkAndHighlight(const QString &name);
    void setup2DMapPage();
    void setupMenuPage();
    void setup3DMapPage();
    bool m_preferenceShown;
    void refreshAfterAdd(int newPoiId);

    MapWidget *m_mapWidget;
    ThumbnailWidget *m_thumbWidget;
    QWidget *m_leftPanel;
    QListWidget *m_destList;
    QLabel *m_titleLabel;
    QStringList m_destinations;
    QLineEdit *m_searchEdit;
    QPushButton *m_searchBtn;
    QCompleter *m_completer;
    QStringList m_allBuildingNames;
    QPushButton *m_addBtn;
    QPushButton *m_cancelBtn;
    QString m_pendingBuildingName;
    RoutePlanner *m_routePlanner;
    QPushButton *m_detourBtn;
    QStringList m_allTags;
    QStackedWidget *m_stackedWidget;
    QWidget *m_menuPage;
    QWidget *m_2dMapPage;
    QWidget *m_3dMapPage;
    Campus3DWidget *m_openGLViewport;

    QString m_startGate;
    QStringList m_gateNames;
    QStringList getAllGates();

    WaypointReorderWidget *m_reorderWidget = nullptr;
    QPushButton *m_reorderBtn = nullptr;
    QStringList m_orderedDestNames;
    QPushButton *m_backTo2DBtn = nullptr;
};

#endif // MAINWINDOW_H