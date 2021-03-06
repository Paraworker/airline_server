#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "connectwindow.h"
#include <vector>

using namespace std;

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
    QByteArray airline_name;
    QByteArray airline_id;
    int server_port;
    QTcpServer* serverSocket;
    QTcpSocket* clientSocket[10];
    QSqlDatabase* database;
    QString database_hostname;
    int database_port;
    QString database_dbname;
    QString database_username;
    QString database_password;


    const QString seat_class[3] = {"头等舱","商务舱","经济舱"};

    bool connect_to_database();
    bool set_socket();
    void list_add(QString s);
    int check_type(QByteArray &text);
    void message_handle(int i);

    void air_query(int i,QByteArray &text);
    void show_seat(int i,QByteArray &text);
    void order(int i,QByteArray &text);
    void refund(int i,QByteArray &text);
    QString get_seat_name(QByteArray a);

    int create_order_number(QByteArray& number);
    QByteArray getRandomNumber();

    int sql_select(vector<QByteArray> &result,QString sql);
    int sql_select(QString sql);
    int sql_operation(QString sql);

};
#endif // MAINWINDOW_H
