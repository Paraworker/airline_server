#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "connectwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startButton_clicked();
    void after_newConnection();

private:
    Ui::MainWindow *ui;
    QTcpServer* serverSocket;
    QTcpSocket* clientSocket[10];
    QSqlDatabase* database;
    QString database_hostname;
    int database_port;
    QString database_dbname;
    QString database_username;
    QString database_password;
    bool connect_to_database();
    bool set_socket();
    int check_type(QString text);
    void message_handle(int i);

    void query(int i,QString text);
    void order(int i,QString text);
    void refund(int i,QString text);

};
#endif // MAINWINDOW_H
