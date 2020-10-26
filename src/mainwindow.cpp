#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    for (int i = 0;i < 10;i++ ) {
        clientSocket[i] = nullptr;
    }
    serverSocket = nullptr;
    database = nullptr;

}

MainWindow::~MainWindow()
{
    delete ui;
    delete database;
    delete serverSocket;
}


void MainWindow::on_startButton_clicked()
{
    if(database == nullptr){
        bool i = connect_to_database();
        if(i != true){
            this->ui->listWidget->addItem("Connect to database failed!");
            return;
        }
        else{
            this->ui->listWidget->addItem("Connect to database succeed!");
        }
    }
    bool k = set_socket();
    if(k != true){
        this->ui->listWidget->addItem("Set socket failed!");
        return;
    }
    else{
        this->ui->listWidget->addItem("Socket listening...");
    }
    this->ui->startButton->setDisabled(true);
    this->ui->startButton->setText("Running");
}

bool MainWindow::connect_to_database(){
    database = new QSqlDatabase;
    *database = QSqlDatabase::addDatabase("QMYSQL");

    ConnectWindow connectwindow;
    connectwindow.set_pointer(&database_hostname,&database_port,&database_dbname,&database_username,&database_password);
    connectwindow.exec();

    database->setHostName(database_hostname);
    database->setPort(database_port);
    database->setDatabaseName(database_dbname);
    database->setUserName(database_username);
    database->setPassword(database_password);
    bool check = database->open();
    if(check){
        return true;
    }
    else{
        delete database;
        database = nullptr;
        return false;
    }
}

bool MainWindow::set_socket(){
    serverSocket = new QTcpServer(this);
    int i = serverSocket->listen(QHostAddress::Any, 18103);
    if(i == false){
        delete serverSocket;
        serverSocket = nullptr;
        return false;
    }
    else {
        connect(serverSocket,&QTcpServer::newConnection,this,&MainWindow::after_newConnection);
        return true;
    }
}

void MainWindow::after_newConnection(){
    int i = 0;
    for(i = 0;i<10;i++){
        if(clientSocket[i] == nullptr){
            clientSocket[i] = serverSocket->nextPendingConnection();
            break;
        }
        if(i == 9){
            this->ui->listWidget->addItem("有新的客户端连接请求，但是连接的客户端数量已满");
            return;
        }
    }
    QString ip = clientSocket[i]->peerAddress().toString();
    unsigned int port = clientSocket[i]->peerPort();
    connect(clientSocket[i],&QTcpSocket::readyRead,this,[=](){
        message_handle(i);
    });

    connect(clientSocket[i],&QTcpSocket::disconnected,this,[=](){
        clientSocket[i] = nullptr;
        this->ui->listWidget->addItem("A client disconnected");
    });

    this->ui->listWidget->addItem("A client connected!      " + QString("[%1]:%2").arg(ip).arg(port));
}

int MainWindow::check_type(QString text){

    return 0;
}

void MainWindow::message_handle(int i){
    QByteArray array = clientSocket[i]->readAll();
    QString text = array.data();
    int type = check_type(text);
    if(type == 0){
        query(i,text);
    }else if (type == 1) {
        order(i,text);
    }else if(type == 2){
        refund(i,text);
    }
}

void MainWindow::query(int i,QString text){

}

void MainWindow::order(int i,QString text){

}

void MainWindow::refund(int i,QString text){

}




