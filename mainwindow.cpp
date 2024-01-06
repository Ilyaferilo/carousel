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
    scene->setBackground(QBrush(Qt::darkCyan));

    ui->graphicsView->setScene(scene);

    // scene->add(new Item("Карусель"));
    // scene->add(new Item("Работает"));
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
    static_cast<Carousel*>(ui->graphicsView->scene())->add(new Item(QString::number(c++)));
}

void MainWindow::on_pushButton_2_clicked()
{
    static_cast<Carousel*>(ui->graphicsView->scene())->reset();
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    static_cast<Carousel*>(ui->graphicsView->scene())->setActive(arg1);
}
