#include "connectwindow.h"
#include "ui_connectwindow.h"
#include <QValidator>

ConnectWindow::ConnectWindow(QWidget *parent) :
    QDialog(parent,Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    ui(new Ui::ConnectWindow)
{
    ui->setupUi(this);
    this->ui->lineEdit_password->setEchoMode(QLineEdit::Password);
    this->ui->lineEdit_port->setValidator(new QIntValidator(0,99999,this));
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
    *database_hostname = this->ui->lineEdit_hostname->text();
    *database_port = this->ui->lineEdit_port->text().toInt();
    *database_dbname = this->ui->lineEdit_dbname->text();
    *database_username = this->ui->lineEdit_username->text();
    *database_password = this->ui->lineEdit_password->text();
    event->accept();
}

void ConnectWindow::set_pointer(QString* hostname,int* port,QString* dbname,QString* username,QString* password){
    database_hostname = hostname;
    database_port = port;
    database_dbname = dbname;
    database_username = username;
    database_password = password;
}
