#include "mainwindow.h"
#include "mapwidget.h"
#include "small_mapwidget.h"
#include "preference_order_dialog.h"
#include "routeplanner.h"
#include "campus3dwidget.h"
#include "detourdialog.h"
#include "waypointreorderwidget.h"

#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QListWidgetItem>
#include <QStringList>
#include <QDebug>
#include <QCompleter>
#include <QLineEdit>
#include <QMessageBox>
#include <QTime>
#include <QApplication>
#include <QInputDialog>
#include <QFile>
#include <QTimer>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),m_preferenceShown(false),m_routePlanner(new RoutePlanner)
{
    resize(1200,800);
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    setupMenuPage();      // 第0页：启动页
    setup2DMapPage();     // 第1页：2D地图页面
    setup3DMapPage();     // 第2页：3D地图页面

    if (m_openGLViewport) {
        auto nodes = m_openGLViewport->getRoadNodesLonLat();
        auto edges = m_openGLViewport->getRoadEdgesInfo();
        std::vector<RoadNodeForPlanner> plannerNodes;
        plannerNodes.reserve(nodes.size());
        for (const auto& n : nodes) {
            plannerNodes.push_back({n.id, n.lon, n.lat});
        }

        std::vector<RoadEdgeForPlanner> plannerEdges;
        plannerEdges.reserve(edges.size());
        for (const auto& e : edges) {
            plannerEdges.push_back({e.from, e.to, e.length});
        }
        m_routePlanner->setRoadNetwork(plannerNodes, plannerEdges);
        qDebug() << "Road network transferred to RoutePlanner:"
                 << plannerNodes.size() << "nodes," << plannerEdges.size() << "edges";
    }

    m_stackedWidget->setCurrentWidget(m_menuPage);
}

// 启动页
void MainWindow::setupMenuPage()
{
    m_menuPage = new QWidget(this);
    QVBoxLayout *menuLayout = new QVBoxLayout(m_menuPage);
    menuLayout->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("北京大学校园3D智能导航系统", m_menuPage);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; margin-bottom: 40px; color: #800000;");
    menuLayout->addWidget(titleLabel);

    QPushButton *enterBtn = new QPushButton("进入地图导航系统", m_menuPage);
    enterBtn->setStyleSheet("padding: 12px 35px; font-size: 16px; font-weight: bold; background-color: #800000; color: white; border-radius: 4px;");
    connect(enterBtn, &QPushButton::clicked, this, &MainWindow::switchTo2DMapView);
    menuLayout->addWidget(enterBtn);

    m_stackedWidget->addWidget(m_menuPage);
}

//2D地图界面
void MainWindow::setup2DMapPage()
{
    m_2dMapPage = new QWidget(this);
    QHBoxLayout *pageLayout = new QHBoxLayout(m_2dMapPage);
    pageLayout->setAlignment(Qt::AlignCenter);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *container = new QWidget;
    container->setFixedSize(800, 800);
    pageLayout->addWidget(container);
    QHBoxLayout *mainLayout = new QHBoxLayout(container);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 左侧面板
    m_leftPanel = new QWidget(container);
    m_leftPanel->setFixedWidth(800 - 595);
    m_leftPanel->setStyleSheet("background-color: white;");
    QVBoxLayout *leftLayout = new QVBoxLayout(m_leftPanel);
    leftLayout->setContentsMargins(10, 10, 10, 10);
    leftLayout->setSpacing(0);
    leftLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // 缩略图
    m_thumbWidget = new ThumbnailWidget(container);
    m_thumbWidget->setFixedSize(220, 220);
    leftLayout->addWidget(m_thumbWidget);
    leftLayout->addSpacing(4);

    // 颜色图例
    QWidget *legendWidget = new QWidget(container);
    QGridLayout *legendLayout = new QGridLayout(legendWidget);
    legendLayout->setSpacing(2);
    legendLayout->setContentsMargins(0, 0, 0, 0);
    auto addLegendItem = [&](int row, int col, const QString &color, const QString &text) {
        QHBoxLayout *itemLayout = new QHBoxLayout;
        itemLayout->setSpacing(4);
        itemLayout->setContentsMargins(0, 0, 0, 0);
        QLabel *dot = new QLabel;
        dot->setFixedSize(10, 10);
        dot->setStyleSheet(QString("background-color: %1; border-radius: 5px;").arg(color));
        QLabel *label = new QLabel(text);
        label->setStyleSheet("color: #1a237e; margin: 0px; padding: 0px;");
        label->setFixedHeight(16);
        itemLayout->addWidget(dot);
        itemLayout->addWidget(label);
        itemLayout->addStretch();
        legendLayout->addLayout(itemLayout, row, col);
    };
    addLegendItem(0, 0, "#cccccc", "未选择");
    addLegendItem(0, 1, "green",   "已选");
    addLegendItem(1, 0, "blue",    "推荐");
    addLegendItem(1, 1, "red",     "高亮/搜索");
    legendWidget->setMaximumHeight(60);
    legendWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftLayout->addWidget(legendWidget);

    // 提示文字
    QLabel *tipLabel = new QLabel("请点击建筑上的圆点选择目的地", container);
    tipLabel->setWordWrap(true);
    tipLabel->setStyleSheet("color: #1a237e; font-size: 12px; margin-top: 0px; margin-bottom: 0px; padding: 0px;");
    tipLabel->setFixedHeight(16);
    leftLayout->addWidget(tipLabel);

    // 已选目的地列表
    m_titleLabel = new QLabel("已选目的地：", container);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setContentsMargins(0, 0, 0, 0);
    m_titleLabel->setFixedHeight(20);
    m_titleLabel->setStyleSheet("color: #1a237e;");
    leftLayout->addWidget(m_titleLabel);

    m_destList = new QListWidget(container);
    m_destList->setMaximumHeight(200);
    m_destList->setSelectionMode(QAbstractItemView::NoSelection);
    m_destList->setAlternatingRowColors(false);
    m_destList->setStyleSheet(
        "QListWidget {"
        "   background-color: rgba(240, 245, 250, 0.6);"
        "   border-radius: 8px;"
        "   border: 1px solid rgba(0, 0, 0, 0.08);"
        "   outline: none;"
        "}"
        "QListWidget::item {"
        "   background-color: rgba(255, 255, 255, 0.5);"
        "   color: #222;"
        "   border-radius: 6px;"
        "   border: 1px solid rgba(0, 0, 0, 0.05);"
        "   padding: 6px;"
        "   margin: 2px 4px;"
        "}"
        );
    leftLayout->addWidget(m_destList);

    // 搜索栏
    QHBoxLayout *searchLayout = new QHBoxLayout;
    m_searchEdit = new QLineEdit(container);
    m_searchEdit->setPlaceholderText("请输入建筑名称");
    // 输入框半透明背景
    m_searchEdit->setStyleSheet(
        "QLineEdit { background-color: rgba(255, 255, 255, 0.6); border-radius: 4px; border: 1px solid rgba(0,0,0,0.1); padding: 4px; }"
        );
    m_searchBtn = new QPushButton("搜索", container);
    m_searchBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(41, 128, 185, 0.5);"
        "   color: white;"
        "   border-radius: 6px;"
        "   border: none;"
        "   padding: 4px 12px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: rgba(41, 128, 185, 0.75); }"
        );
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchBtn);
    leftLayout->addLayout(searchLayout);
    QWidget *spacer1=new QWidget(container);
    spacer1->setFixedHeight(10);
    leftLayout->addWidget(spacer1);

    QWidget *spacerWidget = new QWidget(container);
    spacerWidget->setFixedHeight(15);
    leftLayout->addWidget(spacerWidget);

    QHBoxLayout *actionLayout = new QHBoxLayout;
    actionLayout->setSpacing(30);
    actionLayout->setAlignment(Qt::AlignLeft);
    m_addBtn = new QPushButton("加入", container);
    m_addBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(41, 128, 185, 0.5);"
        "   color: white;"
        "   border-radius: 6px;"
        "   border: none;"
        "   padding: 4px 12px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: rgba(41, 128, 185, 0.75); }"
        );
    m_cancelBtn = new QPushButton("取消", container);
    m_cancelBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(41, 128, 185, 0.5);"
        "   color: white;"
        "   border-radius: 6px;"
        "   border: none;"
        "   padding: 4px 12px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: rgba(41, 128, 185, 0.75); }"
        );
    actionLayout->addWidget(m_addBtn);
    actionLayout->addWidget(m_cancelBtn);
    leftLayout->addLayout(actionLayout);
    QWidget *spacer2=new QWidget(container);
    spacer2->setFixedHeight(10);
    leftLayout->addWidget(spacer2);
    m_addBtn->hide();
    m_cancelBtn->hide();

    //沿途推荐按钮
    m_detourBtn = new QPushButton("沿途推荐", m_2dMapPage);
    m_detourBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(41, 128, 185, 0.5);"
        "   color: white;"
        "   border-radius: 6px;"
        "   border: none;"
        "   padding: 4px 12px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: rgba(41, 128, 185, 0.75); }"
        );
    leftLayout->addWidget(m_detourBtn);
    QWidget *spacer3=new QWidget(container);
    spacer3->setFixedHeight(10);
    leftLayout->addWidget(spacer3);
    connect(m_detourBtn, &QPushButton::clicked, this, &MainWindow::onDetourRecommend);

    //确认按钮
    QPushButton *confirmBtn = new QPushButton("确认，进入3D界面", container);
    confirmBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(41, 128, 185, 0.5);"
        "   color: white;"
        "   border-radius: 6px;"
        "   border: none;"
        "   padding: 4px 12px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: rgba(41, 128, 185, 0.75); }"
        );
    leftLayout->addWidget(confirmBtn);
    connect(confirmBtn, &QPushButton::clicked, this, &MainWindow::onConfirmDestinations);

    // 右侧地图
    m_mapWidget = new MapWidget(container);
    m_mapWidget->setFixedSize(595, 800);
    mainLayout->addWidget(m_leftPanel);
    mainLayout->addWidget(m_mapWidget);

    // 信号连接
    connect(m_mapWidget, &MapWidget::mapImageLoaded, this, [this](){
        QSize imgSize = m_mapWidget->getMapImageSize();
        if (imgSize.isEmpty()) return;
        int thumbWidth = m_leftPanel->width() * 0.8;
        if (thumbWidth < 100) thumbWidth = 100;
        int thumbHeight = thumbWidth * imgSize.height() / imgSize.width();
        m_thumbWidget->setFixedSize(thumbWidth, thumbHeight);
        m_thumbWidget->setMapImage(m_mapWidget->getMapImage());
        m_mapWidget->fitToWindow();
    });
    connect(m_mapWidget, &MapWidget::viewportChanged, m_thumbWidget, &ThumbnailWidget::setViewport);
    connect(m_thumbWidget, &ThumbnailWidget::viewportRequested, m_mapWidget, &MapWidget::moveToCenter);
    connect(m_mapWidget, &MapWidget::buildingClicked, this, &MainWindow::onBuildingClicked);
    connect(m_mapWidget, &MapWidget::highlightBuildingClicked, this, [this](const QString &name){
        if (m_destinations.contains(name)) {
            QMessageBox::information(this, "提示", QString("“%1”已经在已选列表中").arg(name));
        } else if (m_destinations.size() >= 10) {
            QMessageBox::warning(this, "限制", "最多只能选择10个目的地");
        } else {
            m_destinations.append(name);
            m_mapWidget->selectBuilding(name);
            updateDestinationList();
            m_searchEdit->clear();
        }
        m_mapWidget->clearHighlight();
        m_addBtn->hide();
        m_cancelBtn->hide();
        m_pendingBuildingName.clear();
    });
    connect(m_searchBtn, &QPushButton::clicked, this, &MainWindow::onSearchClicked);
    connect(m_addBtn, &QPushButton::clicked, this, &MainWindow::onAddButtonClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &MainWindow::onCancelButtonClicked);

    // 加载数据
    m_mapWidget->loadMapImage(":/images/map4.jpg");
    m_mapWidget->loadBuildings(":/data/named_features.csv");
    m_mapWidget->addReferencePoint(116.298592, 39.993295, 19, 548);
    m_mapWidget->addReferencePoint(116.306212, 39.986233, 825, 1484);
    m_mapWidget->addReferencePoint(116.305782, 39.992636, 800, 676);
    m_mapWidget->addReferencePoint(116.301696, 39.986257, 360, 1479);
    m_mapWidget->addReferencePoint(116.303938, 39.990529, 612, 928);
    m_mapWidget->computeTransform();

    QSet<QString> allTagsSet;
    for (const auto &b : m_mapWidget->getAllBuildings()) {
        for (const QString &tag : b.tags) {
            allTagsSet.insert(tag);
        }
    }
    m_allTags = allTagsSet.values();
    m_gateNames = getAllGates();

    // 路径规划器加载
    QString csvPath = ":/data/named_features.csv";
    if (!m_routePlanner->loadFromCSV(csvPath.toStdString())) {
        qDebug() << "路径规划器加载失败" << csvPath;
    }

    setupCompleter();

    m_stackedWidget->addWidget(m_2dMapPage);
}

//3D地图界面
void MainWindow::setup3DMapPage()
{
    m_3dMapPage = new QWidget(this);
    QHBoxLayout *mapHorizontalLayout = new QHBoxLayout(m_3dMapPage);
    mapHorizontalLayout->setContentsMargins(0, 0, 0, 0);
    mapHorizontalLayout->setSpacing(0);

    m_openGLViewport = new Campus3DWidget(m_3dMapPage);
    mapHorizontalLayout->addWidget(m_openGLViewport, 1);

    //切换2d界面按钮
    m_backTo2DBtn = new QPushButton("切换2D界面", m_3dMapPage);
    m_backTo2DBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(20,30,45,210);"
        "  color: white; border: 1px solid rgba(255,255,255,60);"
        "  border-radius: 6px; padding: 8px 14px;"
        "  font-size: 12px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: rgba(128,0,0,210); }"
        );
    m_backTo2DBtn->setFixedSize(100, 36);
    m_backTo2DBtn->raise();
    m_backTo2DBtn->hide();
    connect(m_backTo2DBtn, &QPushButton::clicked, this, &MainWindow::switchTo2DMapView);

    //调整顺序
    m_reorderBtn = new QPushButton("调整顺序", m_3dMapPage);
    m_reorderBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(20,30,45,210);"
        "  color: white; border: 1px solid rgba(255,255,255,60);"
        "  border-radius: 6px; padding: 8px 14px;"
        "  font-size: 12px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: rgba(128,0,0,210); }"
        );
    m_reorderBtn->setFixedSize(100, 36);
    m_reorderBtn->raise();
    m_reorderBtn->hide();

    connect(m_reorderBtn, &QPushButton::clicked,
            this, &MainWindow::onShowReorderPanel);

    m_stackedWidget->addWidget(m_3dMapPage);
}

// 切换页面槽函数
void MainWindow::switchTo2DMapView()
{
    m_stackedWidget->setCurrentWidget(m_2dMapPage);

    if (!m_preferenceShown) {
        m_preferenceShown = true;

        QSet<QString> allTagsSet;
        for (const auto &b : m_mapWidget->getAllBuildings()) {
            for (const QString &tag : b.tags) {
                allTagsSet.insert(tag);
            }
        }
        QStringList allTags = allTagsSet.values();
        allTags.removeAll("起点");

        PreferenceOrderDialog dlg(allTags, m_gateNames, this);
        if (dlg.exec() == QDialog::Accepted) {
            QStringList orderedTags = dlg.getOrderedTags();
            m_startGate = dlg.getSelectedGate();
            int day = dlg.getDayOfWeek();
            QTime startTime = dlg.getStartTime();
            Q_UNUSED(day);
            Q_UNUSED(startTime);
            applyPreference(orderedTags);
        } else {
            applyPreference(QStringList());
            if (m_startGate.isEmpty()) m_startGate = "北大西门";
        }
    }
}

void MainWindow::switchTo3DView()
{
    m_stackedWidget->setCurrentWidget(m_3dMapPage);

    if (m_backTo2DBtn) {
        int x = 12;
        int y = m_3dMapPage->height() - m_backTo2DBtn->height() - 12;
        m_backTo2DBtn->move(x, y);
        m_backTo2DBtn->raise();
        m_backTo2DBtn->show();
    }

    if (m_reorderBtn && m_reorderBtn->isVisible()) {
        int x = 12;
        int y = 12;
        m_reorderBtn->move(x, y);
        m_reorderBtn->raise();
    }
}

void MainWindow::switchToMenuView()
{
    m_stackedWidget->setCurrentWidget(m_menuPage);
}

void MainWindow::updateDestinationList()
{
    m_destList->clear();
    for (int i = 0; i < m_destinations.size(); ++i) {
        const QString &name = m_destinations[i];
        QWidget *itemWidget = new QWidget;
        QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
        itemLayout->setContentsMargins(5, 2, 5, 2);
        itemLayout->setSpacing(5);

        QLabel *indexLabel = new QLabel(QString("%1.").arg(i+1));
        indexLabel->setFixedWidth(30);
        QLabel *nameLabel = new QLabel(name);
        nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        nameLabel->setWordWrap(true);

        QPushButton *delBtn = new QPushButton("删除");
        delBtn->setFixedSize(30, 25);
        connect(delBtn, &QPushButton::clicked, this, [this, i]() {
            onDeleteDestination(i);
        });

        itemLayout->addWidget(indexLabel);
        itemLayout->addWidget(nameLabel);
        itemLayout->addWidget(delBtn);

        QListWidgetItem *listItem = new QListWidgetItem(m_destList);
        m_destList->setItemWidget(listItem, itemWidget);
        listItem->setSizeHint(QSize(0, 60));
    }
}

bool MainWindow::onBuildingClicked(const QString &name, const QPointF &)
{
    if (m_destinations.contains(name)) {
        QMessageBox::information(this, "提示", QString("“%1”已经在已选列表中").arg(name));
        return false;
    }
    if (m_destinations.size() >= 10) {
        QMessageBox::warning(this, "限制", "最多只能选择10个目的地");
        return false;
    }
    m_destinations.append(name);
    m_mapWidget->selectBuilding(name);
    updateDestinationList();
    m_searchEdit->clear();
    return true;
}

void MainWindow::onDeleteDestination(int index)
{
    if (index < 0 || index >= m_destinations.size()) return;
    QString name = m_destinations[index];
    m_destinations.removeAt(index);
    m_mapWidget->deselectBuilding(name);
    updateDestinationList();
}

void MainWindow::onSearchClicked()
{
    QString text = m_searchEdit->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::information(this, "提示", "请输入建筑名称");
        return;
    }

    QString matchedName;
    for (const QString &name : m_allBuildingNames) {
        if (name.compare(text, Qt::CaseInsensitive) == 0) {
            matchedName = name;
            break;
        }
    }

    if (matchedName.isEmpty()) {
        QMessageBox::warning(this, "未找到", QString("没有找到名为“%1”的建筑").arg(text));
        return;
    }

    checkAndHighlight(matchedName);

}

void MainWindow::checkAndHighlight(const QString &name)
{
    m_addBtn->hide();
    m_cancelBtn->hide();
    m_pendingBuildingName.clear();
    if (m_destinations.contains(name)) {
        QMessageBox::information(this, "提示", QString("“%1”已经在已选列表中").arg(name));
        return;
    }

    m_mapWidget->highlightBuilding(name);
    m_pendingBuildingName = name;
    m_addBtn->show();
    m_cancelBtn->show();
}

void MainWindow::onAddButtonClicked()
{
    if (!m_pendingBuildingName.isEmpty()) {
        bool success = onBuildingClicked(m_pendingBuildingName, QPointF());
        m_mapWidget->clearHighlight();
        m_addBtn->hide();
        m_cancelBtn->hide();
        m_pendingBuildingName.clear();
        if(success){
            m_searchEdit->clear();
        }
    }
}

void MainWindow::onCancelButtonClicked()
{
    m_mapWidget->clearHighlight();
    m_addBtn->hide();
    m_cancelBtn->hide();
    m_pendingBuildingName.clear();
}

void MainWindow::onCompleterActivated(const QString &text)
{
    checkAndHighlight(text);
}

void MainWindow::setupCompleter()
{
    m_allBuildingNames = m_mapWidget->getAllBuildingNames();
    if (m_allBuildingNames.isEmpty()) {
        qDebug() << "没有建筑数据，无法设置自动补全";
        return;
    }
    m_completer = new QCompleter(m_allBuildingNames, this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_searchEdit->setCompleter(m_completer);
    QListView *popupView = qobject_cast<QListView*>(m_completer->popup());
    if (popupView) {
        popupView->setWordWrap(true);
        popupView->setUniformItemSizes(false);
        popupView->setStyleSheet("QListView::item { white-space: normal; padding: 2px; }");
    }

    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &MainWindow::onCompleterActivated);
}

void MainWindow::applyPreference(const QStringList &selectedTags)
{
    if (selectedTags.isEmpty()) {
        m_mapWidget->setRecommendedBuildings(QStringList());
        return;
    }

    QVector<BuildingInfo> allBuildings = m_mapWidget->getBuildings();
    // 存储每个建筑的 (最高优先级索引, 建筑名称)
    QVector<QPair<int, QString>> buildingsWithPriority;

    for (const auto &b : allBuildings) {
        int bestPriority = -1;
        for (int i = 0; i < selectedTags.size(); ++i) {
            if (b.tags.contains(selectedTags[i])) {
                bestPriority = i;
                break; // 找到第一个匹配的标签即为最高优先级
            }
        }
        if (bestPriority != -1) {
            buildingsWithPriority.append({bestPriority, b.name});
        }
    }

    // 按优先级索引升序分组
    std::sort(buildingsWithPriority.begin(), buildingsWithPriority.end(),
              [](const QPair<int,QString> &a, const QPair<int,QString> &b) {
                  if (a.first != b.first) return a.first < b.first;
                  return a.second < b.second;
              });


    QVector<int> targetCounts = {8, 5, 2};
    QStringList recommendedNames;
    int currentGroup = 0;
    int groupStart = 0;
    int groupEnd = 0;

    // 找到每个优先级组的范围
    while (currentGroup < selectedTags.size() && recommendedNames.size() < 15) {
        groupEnd = groupStart;
        while (groupEnd < buildingsWithPriority.size() &&
               buildingsWithPriority[groupEnd].first == currentGroup) {
            ++groupEnd;
        }
        if (groupStart == groupEnd) {
            ++currentGroup;
            continue;
        }

        int need = (currentGroup < targetCounts.size()) ? targetCounts[currentGroup] : 1;
        int take = qMin(need, groupEnd - groupStart);
        for (int i = 0; i < take && recommendedNames.size() < 15; ++i) {
            recommendedNames.append(buildingsWithPriority[groupStart + i].second);
        }
        groupStart = groupEnd;
        ++currentGroup;
    }

    for (int i = groupStart; i < buildingsWithPriority.size() && recommendedNames.size() < 15; ++i) {
        recommendedNames.append(buildingsWithPriority[i].second);
    }

    m_mapWidget->setRecommendedBuildings(recommendedNames);
}

void MainWindow::onConfirmDestinations()
{
    if (m_destinations.isEmpty()) {
        QMessageBox::warning(this, "提示", "请至少选择一个目的地");
        return;
    }

    QString startName = m_startGate;
    std::string startNameStd = startName.toUtf8().constData();
    int startId = m_routePlanner->findPOIByName(startNameStd);
    if (startId == -1) {
        QMessageBox::warning(this, "错误", "未找到起点: " + startName);
        return;
    }

    std::vector<int> destIds;
    for (const QString& name : m_destinations) {
        std::string nameStd = name.toUtf8().constData();
        int id = m_routePlanner->findPOIByName(nameStd);
        if (id != -1) destIds.push_back(id);
        else qDebug() << "未找到目的地:" << name;
    }

    if (destIds.empty()) {
        QMessageBox::warning(this, "提示", "没有有效的目的地");
        return;
    }

    double totalTime = 0, totalDistance = 0;
    std::vector<int> optimalOrder = m_routePlanner->planRoute(startId, destIds, totalTime, totalDistance);
    QVector<QString> orderedNames;
    orderedNames.append(startName);
    for (int id : optimalOrder) {
        orderedNames.append(QString::fromStdString(m_routePlanner->getPOIName(id)));
    }

    m_orderedDestNames.clear();
    for (int i = 1; i < orderedNames.size(); ++i) {
        m_orderedDestNames.append(orderedNames[i]);
    }
    QVector<QPointF> fullPathPoints;

    for (int i = 0; i < orderedNames.size() - 1; ++i) {
        QString from = orderedNames[i];
        QString to = orderedNames[i+1];
        std::vector<std::vector<double>> segmentPoints = m_routePlanner->getPathTurningPoints(
            from.toStdString(), to.toStdString());

        if (segmentPoints.empty()) {
            qDebug() << "Warning: No road path from" << from << "to" << to << ", using straight line";
            // 回退：直接连接两个POI的直线
            double lon1, lat1, lon2, lat2;
            int id_from = m_routePlanner->findPOIByName(from.toStdString());
            int id_to   = m_routePlanner->findPOIByName(to.toStdString());
            if (id_from != -1 && id_to != -1) {
                m_routePlanner->getBuildingLonLat(id_from, lon1, lat1);
                m_routePlanner->getBuildingLonLat(id_to,   lon2, lat2);
                if (i == 0) {
                    fullPathPoints.append(QPointF(lon1, lat1));
                }
                fullPathPoints.append(QPointF(lon2, lat2));
            }
            continue;
        }

        // 将路段点添加到总路径
        for (size_t j = 0; j < segmentPoints.size(); ++j) {
            QPointF pt(segmentPoints[j][0], segmentPoints[j][1]);
            if (fullPathPoints.isEmpty() || fullPathPoints.last() != pt) {
                fullPathPoints.append(pt);
            }
        }
    }

    // 将经纬度路径点转换为2D地图上的像素点
    QVector<QPointF> routePoints;
    for (const QPointF& ll : fullPathPoints) {
        QPointF px = m_mapWidget->lonLatToPx(ll.x(), ll.y());
        if (px.x() >= 0 && px.y() >= 0) {
            routePoints.append(px);
        }
    }

    // 显示总时间、距离
    QString info = QString("总路程: %1 米\n总时间（含停留）: %2 分钟")
                       .arg((int)totalDistance)
                       .arg(totalTime, 0, 'f', 1);
    QMessageBox::information(this, "路径规划结果", info);

    m_mapWidget->setRoutePoints(routePoints);
    if (!fullPathPoints.isEmpty()) {
        m_openGLViewport->setNavigationPath(fullPathPoints);
    } else {
        qDebug() << "No path points generated, using fallback";
        QVector<QPointF> fallback;
        double startLon, startLat;
        m_routePlanner->getBuildingLonLat(startId, startLon, startLat);
        fallback.append(QPointF(startLon, startLat));
        for (int id : optimalOrder) {
            double lon, lat;
            m_routePlanner->getBuildingLonLat(id, lon, lat);
            fallback.append(QPointF(lon, lat));
        }
        m_openGLViewport->setNavigationPath(fallback);
    }

    m_reorderBtn->show();
    m_backTo2DBtn->show();
    switchTo3DView();
}

void MainWindow::onDetourRecommend()
{
    // 检查是否有目的地
    if (m_destinations.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择至少一个目的地");
        return;
    }

    if (m_allTags.isEmpty()) {
        QMessageBox::warning(this, "提示", "没有可用的标签数据");
        return;
    }
    QStringList filteredTags = m_allTags;
    filteredTags.removeAll("起点");

    // 弹出下拉选择框，让用户选择标签
    bool ok;
    QString tag = QInputDialog::getItem(this, "沿途搜索", "请选择标签:", filteredTags, 0, false, &ok);
    if (!ok || tag.isEmpty()) return;

    // 判断是否美食标签（需要显示食堂信息）
    bool showCanteen = (tag == "美食");

    // 获取起点 ID（使用用户选择的起点）
    QString startName = m_startGate;
    int startId = m_routePlanner->findPOIByName(startName.toUtf8().constData());
    if (startId == -1) {
        QMessageBox::warning(this, "错误", "未找到起点: " + startName);
        return;
    }

    // 获取当前已选目的地的 ID 列表
    std::vector<int> destIds;
    for (const QString& name : m_destinations) {
        int id = m_routePlanner->findPOIByName(name.toUtf8().constData());
        if (id != -1) destIds.push_back(id);
        else qDebug() << "未找到目的地:" << name;
    }
    if (destIds.empty()) return;

    // 计算当前最优顺序
    double dummy;
    std::vector<int> optimalOrder = m_routePlanner->planRoute(startId, destIds, dummy, dummy);
    std::vector<int> currentPath;
    currentPath.push_back(startId);
    currentPath.insert(currentPath.end(), optimalOrder.begin(), optimalOrder.end());

    // 调用沿途推荐接口
    std::vector<DetourRecommendation> recs = m_routePlanner->searchByTagAndSortByDetour(currentPath, tag.toStdString());

    if (recs.empty()) {
        QMessageBox::information(this, "提示", QString("未找到沿途包含标签“%1”的景点").arg(tag));
        return;
    }

    // 显示推荐对话框
    DetourDialog dlg(recs, showCanteen, this);
    if (dlg.exec() == QDialog::Accepted) {
        int newId = dlg.getSelectedPoiId();
        if (newId != -1) {
            refreshAfterAdd(newId);
        }
    }
}

void MainWindow::refreshAfterAdd(int newPoiId)
{
    std::string nameStd = m_routePlanner->getPOIName(newPoiId);
    QString newName = QString::fromStdString(nameStd);
    if (m_destinations.contains(newName)) {
        QMessageBox::information(this, "提示", "该景点已在列表中");
        return;
    }
    if (m_destinations.size() >= 10) {
        QMessageBox::warning(this, "限制", "最多10个目的地");
        return;
    }

    m_destinations.append(newName);
    m_mapWidget->selectBuilding(newName);
    updateDestinationList();

    // 重新规划
    QString startName = m_startGate;
    int startId = m_routePlanner->findPOIByName(startName.toUtf8().constData());
    std::vector<int> destIds;
    for (const QString& name : m_destinations) {
        int id = m_routePlanner->findPOIByName(name.toUtf8().constData());
        if (id != -1) destIds.push_back(id);
    }
    double totalTime, totalDistance;
    std::vector<int> newOrder = m_routePlanner->planRoute(startId, destIds, totalTime, totalDistance);
    m_orderedDestNames.clear();
    for (int id : newOrder) {
        m_orderedDestNames.append(QString::fromStdString(m_routePlanner->getPOIName(id)));
    }

    // 更新3D路线
    QVector<QString> orderedNames;
    orderedNames.append(startName);
    for (int id : newOrder) {
        orderedNames.append(QString::fromStdString(m_routePlanner->getPOIName(id)));
    }
    QVector<QPointF> fullPathPoints;
    for (int i = 0; i < orderedNames.size() - 1; ++i) {
        QString from = orderedNames[i];
        QString to = orderedNames[i+1];
        std::vector<std::vector<double>> segmentPoints = m_routePlanner->getPathTurningPoints(
            from.toStdString(), to.toStdString());

        if (segmentPoints.empty()) {
            // 回退：直线连接
            double lon1, lat1, lon2, lat2;
            int id_from = m_routePlanner->findPOIByName(from.toStdString());
            int id_to   = m_routePlanner->findPOIByName(to.toStdString());
            if (id_from != -1 && id_to != -1) {
                m_routePlanner->getBuildingLonLat(id_from, lon1, lat1);
                m_routePlanner->getBuildingLonLat(id_to,   lon2, lat2);
                if (i == 0) fullPathPoints.append(QPointF(lon1, lat1));
                fullPathPoints.append(QPointF(lon2, lat2));
            }
            continue;
        }

        for (size_t j = 0; j < segmentPoints.size(); ++j) {
            QPointF pt(segmentPoints[j][0], segmentPoints[j][1]);
            if (fullPathPoints.isEmpty() || fullPathPoints.last() != pt)
                fullPathPoints.append(pt);
        }
    }

    // 同步更新2D地图路线
    QVector<QPointF> routePoints;
    for (const QPointF& ll : fullPathPoints) {
        QPointF px = m_mapWidget->lonLatToPx(ll.x(), ll.y());
        if (px.x() >= 0 && px.y() >= 0)
            routePoints.append(px);
    }
    m_mapWidget->setRoutePoints(routePoints);

    if (!fullPathPoints.isEmpty()) {
        m_openGLViewport->setNavigationPath(fullPathPoints);
    } else {
        QVector<QPointF> fallback;
        for (const QString& name : orderedNames) {
            int id = m_routePlanner->findPOIByName(name.toStdString());
            if (id != -1) {
                double lon, lat;
                m_routePlanner->getBuildingLonLat(id, lon, lat);
                fallback.append(QPointF(lon, lat));
            }
        }
        m_openGLViewport->setNavigationPath(fallback);
    }

    QMessageBox::information(this, "成功", QString("已添加 %1 并重新规划路线").arg(newName));
}

MainWindow::~MainWindow()
{
    delete m_routePlanner;
}


QStringList MainWindow::getAllGates()
{
    QStringList gates;
    QFile file(":/data/gates.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开门数据文件：:/data/gates.csv";
        return gates;
    }
    QTextStream in(&file);
    if (!in.atEnd()) in.readLine();
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty()) continue;
        QStringList fields = line.split(',');
        if (fields.size() >= 1) {
            QString name = fields[0].trimmed();
            if (name != "邱门") {
                gates.append(name);
            }
        }
    }
    file.close();
    return gates;
}

void MainWindow::onShowReorderPanel()
{
    if (m_orderedDestNames.isEmpty()) return;
    int startId = m_routePlanner->findPOIByName(m_startGate.toStdString());
    double startLon, startLat;
    if (!m_routePlanner->getBuildingLonLat(startId, startLon, startLat)) return;
    QVector<QPointF> poiPoints;
    poiPoints.append(QPointF(startLon, startLat));
    for (const QString& name : m_orderedDestNames) {
        int id = m_routePlanner->findPOIByName(name.toStdString());
        if (id != -1) {
            double lon, lat;
            m_routePlanner->getBuildingLonLat(id, lon, lat);
            poiPoints.append(QPointF(lon, lat));
        }
    }
    QStringList names;
    names.append(m_startGate);
    names.append(m_orderedDestNames);

    if (m_reorderWidget) {
        m_reorderWidget->deleteLater();
        m_reorderWidget = nullptr;
    }

    m_reorderWidget = new WaypointReorderWidget(poiPoints, names, m_3dMapPage);
    m_reorderWidget->resize(m_3dMapPage->size());
    m_reorderWidget->raise();
    m_reorderWidget->show();

    connect(m_reorderWidget, &WaypointReorderWidget::pathReordered,
            this, &MainWindow::onPathReordered);
    connect(m_reorderWidget, &WaypointReorderWidget::reorderCancelled,
            this, [this]() {
                if (m_reorderWidget) m_reorderWidget->hide();
            });
}

void MainWindow::onPathReordered(const QVector<QPointF> &newPath)
{
    Q_UNUSED(newPath);
    if (!m_reorderWidget) return;
    QStringList newNames = m_reorderWidget->currentNames();
    if (newNames.size() < 2) return;
    // 更新起点和目的地列表
    m_startGate = newNames[0];
    m_destinations.clear();
    for (int i = 1; i < newNames.size(); ++i)
        m_destinations.append(newNames[i]);
    m_orderedDestNames = m_destinations;
    QVector<QString> orderedNames;
    orderedNames.append(m_startGate);
    orderedNames.append(m_orderedDestNames);
    QVector<QPointF> fullPathPoints;
    for (int i = 0; i < orderedNames.size() - 1; ++i) {
        QString from = orderedNames[i];
        QString to = orderedNames[i+1];
        std::vector<std::vector<double>> segmentPoints = m_routePlanner->getPathTurningPoints(
            from.toStdString(), to.toStdString());

        if (segmentPoints.empty()) {
            // 直线回退
            double lon1, lat1, lon2, lat2;
            int id_from = m_routePlanner->findPOIByName(from.toStdString());
            int id_to   = m_routePlanner->findPOIByName(to.toStdString());
            if (id_from != -1 && id_to != -1) {
                m_routePlanner->getBuildingLonLat(id_from, lon1, lat1);
                m_routePlanner->getBuildingLonLat(id_to,   lon2, lat2);
                if (i == 0) fullPathPoints.append(QPointF(lon1, lat1));
                fullPathPoints.append(QPointF(lon2, lat2));
            }
            continue;
        }

        for (size_t j = 0; j < segmentPoints.size(); ++j) {
            QPointF pt(segmentPoints[j][0], segmentPoints[j][1]);
            if (fullPathPoints.isEmpty() || fullPathPoints.last() != pt)
                fullPathPoints.append(pt);
        }
    }

    // 更新 2D 地图上的路线
    QVector<QPointF> routePoints;
    for (const QPointF& ll : fullPathPoints) {
        QPointF px = m_mapWidget->lonLatToPx(ll.x(), ll.y());
        if (px.x() >= 0 && px.y() >= 0)
            routePoints.append(px);
    }
    m_mapWidget->setRoutePoints(routePoints);

    // 更新 3D 视图
    if (!fullPathPoints.isEmpty())
        m_openGLViewport->setNavigationPath(fullPathPoints);
    else {
        // 回退：直接用 POI 连线
        QVector<QPointF> fallback;
        for (const QString& name : orderedNames) {
            int id = m_routePlanner->findPOIByName(name.toStdString());
            if (id != -1) {
                double lon, lat;
                m_routePlanner->getBuildingLonLat(id, lon, lat);
                fallback.append(QPointF(lon, lat));
            }
        }
        m_openGLViewport->setNavigationPath(fallback);
    }
    if (m_reorderWidget) m_reorderWidget->hide();
}