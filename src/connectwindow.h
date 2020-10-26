#ifndef CONNECTWINDOW_H
#define CONNECTWINDOW_H

#include <QDialog>
#include <QCloseEvent>

namespace Ui {
class ConnectWindow;
}

class ConnectWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectWindow(QWidget *parent = nullptr);
    ~ConnectWindow();
    void set_pointer(QString* hostname,int* port,QString* dbname,QString* username,QString* password);

private:
    Ui::ConnectWindow *ui;
    QString* database_hostname;
    int* database_port;
    QString* database_dbname;
    QString* database_username;
    QString* database_password;
    void closeEvent( QCloseEvent * event);

private slots:
    void on_pushButton_connect_clicked();
};

#endif // CONNECTWINDOW_H
