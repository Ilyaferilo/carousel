#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qgraphicsscene.h"
#include <QGraphicsRectItem>
#include <QTimer>
#include <qgraphicsitem.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "item.h"
#include "carousel/carousel.h"

class PixItem
  : virtual public QGraphicsObject
  , virtual public QGraphicsPixmapItem
{
public:
    QRectF boundingRect() const
    {
        return QGraphicsPixmapItem::boundingRect();
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr)
    {
        painter->setRenderHint(QPainter::Antialiasing);
        QGraphicsPixmapItem::paint(painter, option, widget);
    }
};

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto scene = new Carousel(this);
    scene->setMargin(20);
    scene->setBackground(QBrush(Qt::black));

    QTimer::singleShot(1, this, [this] {
        resize(400, 800);
    });
    ui->graphicsView->setScene(scene);
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
    static int c;
    // static_cast<Carousel*>(ui->graphicsView->scene())->setItemSize({450, 150});
    static_cast<Carousel*>(ui->graphicsView->scene())->add(new Item(QString::number(c++)));
}

void MainWindow::on_pushButton_2_clicked()
{
    static_cast<Carousel*>(ui->graphicsView->scene())->reset();
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    static_cast<Carousel*>(ui->graphicsView->scene())->setActive(arg1);
    // static_cast<Carousel*>(ui->graphicsView->scene())->setMargin(arg1);
}
