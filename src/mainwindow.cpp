#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    airline_name = "中国南方航空";
    server_port = 181035;
    //根据不同航空公司修改

    this->ui->label_title->setText("Airline Server (" + airline_name + ")");
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
        this->ui->listWidget->addItem("A client connected!");

        //this->ui->listWidget->addItem("查询结果已发回");
        //this->ui->listWidget->addItem("订票结果已发回");
        //this->ui->listWidget->addItem("退票结果已发回");
    }
    this->ui->startButton->setDisabled(true);
    this->ui->startButton->setText("Running");
}

bool MainWindow::connect_to_database(){
    ConnectWindow connectwindow;
    connectwindow.set_pointer(&database_hostname,&database_port,&database_dbname,&database_username,&database_password);
    connectwindow.exec();

    database = new QSqlDatabase;
    *database = QSqlDatabase::addDatabase("QMYSQL");
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
    int i = serverSocket->listen(QHostAddress::Any, server_port);
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

int MainWindow::check_type(QString &text){
    if(text.startsWith("0#")){
        text =text.mid(2);
        return 0;
    } else if(text.startsWith("1#")){
        text =text.mid(2);
        return 1;
    }else if(text.startsWith("2#")){
        text =text.mid(2);
        return 2;
    }
}

void MainWindow::message_handle(int i){
    QByteArray array = clientSocket[i]->readAll();
    QString text = array.data();
    int type = check_type(text);
    if(type == 0){
        air_query(i,text);
    }else if (type == 1) {
        order(i,text);
    }else if(type == 2){
        refund(i,text);
    }
}

void MainWindow::air_query(int i,QString &text){
    this->ui->listWidget->addItem("[查询请求] 杭州 -> 武汉 2020/10/26 商务舱");


}

void MainWindow::order(int i,QString &text){
    this->ui->listWidget->addItem("[订票请求] 杭州 -> 武汉 2020/10/26 17:20-20:30 商务舱 座位：17");

}

void MainWindow::refund(int i,QString &text){
    this->ui->listWidget->addItem("[退票请求] 订单号：000000000023");

}




