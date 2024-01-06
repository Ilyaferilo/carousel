#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qgraphicsscene.h"
#include "carousel.h"
#include <QGraphicsRectItem>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto scene = new Carousel(this);
    scene->setPadding(20);

    ui->graphicsView->setScene(scene);

    //    auto x = scene->addRect(QRect(0, 0, 120, 250), {}, QBrush(Qt::darkGreen));

    //    new QGraphicsSimpleTextItem("1", x);
    //    new QGraphicsSimpleTextItem("2", x);
    //    new QGraphicsSimpleTextItem("3", x);

    //    scene->setForegroundBrush(QBrush(Qt::darkYellow));

    //    auto timer = new QTimer(this);
    //    connect(timer, &QTimer::timeout, this, [=] {
    //        const auto colided = x->collidingItems();
    //        if (!colided.isEmpty()) {
    //            qDebug() << colided;
    //        }
    //    });
    //    timer->start(100);

    //    for (auto i : scene->items()) {
    //        i->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    //    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    static_cast<Carousel*>(ui->graphicsView->scene())
      ->setSceneRectangle(ui->graphicsView->rect().adjusted(10, 10, -10, -10));
}

void MainWindow::on_pushButton_clicked()
{
    static_cast<Carousel*>(ui->graphicsView->scene())->add();
}

void MainWindow::on_pushButton_2_clicked()
{
    static_cast<Carousel*>(ui->graphicsView->scene())->reset();
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    static_cast<Carousel*>(ui->graphicsView->scene())->setActive(arg1);
}
