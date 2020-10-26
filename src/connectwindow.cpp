#include "connectwindow.h"
#include "ui_connectwindow.h"

ConnectWindow::ConnectWindow(QWidget *parent) :
    QDialog(parent,Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    ui(new Ui::ConnectWindow)
{
    ui->setupUi(this);
}

ConnectWindow::~ConnectWindow()
{
    delete ui;
}

void ConnectWindow::on_pushButton_connect_clicked()
{
    this->close();
}

void ConnectWindow::closeEvent(QCloseEvent *event){
    *database_hostname = this->ui->textEdit_hostname->toPlainText();
    *database_dbname = this->ui->textEdit_dbname->toPlainText();
    *database_username = this->ui->textEdit_username->toPlainText();
    *database_password = this->ui->textEdit_password->toPlainText();
    event->accept();
}

void ConnectWindow::set_pointer(QString* hostname,QString* dbname,QString* username,QString* password){
    database_hostname = hostname;
    database_dbname = dbname;
    database_username = username;
    database_password = password;
}
