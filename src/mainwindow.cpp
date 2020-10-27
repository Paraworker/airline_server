#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    airline_name = "中国南方航空";
    server_port = 18103;
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
            QString addflag = "0%" + airline_name;
            clientSocket[i]->write(addflag.toUtf8().data());    //发回航空公司名字
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

int MainWindow::check_type(QByteArray &text){
    if(text.startsWith("0#")){
        text =text.mid(2);
        return 0;
    }
    else if(text.startsWith("1#")){
        text =text.mid(2);
        return 1;
    }
    else if(text.startsWith("2#")){
        text =text.mid(2);
        return 2;
    }
    else{
        return -1;
    }
}

void MainWindow::message_handle(int i){
    QByteArray array = clientSocket[i]->readAll();
    int type = check_type(array);
    if(type == 0){
        air_query(i,array);
    }else if (type == 1) {
        order(i,array);
    }else if(type == 2){
        refund(i,array);
    }
}

void MainWindow::air_query(int i,QByteArray &text){
    QByteArray info[4];    //Starting Terminal date seat_class
    char* str = text.data();
    int tag = 0;
    while(*str != '\0'){
        if(*str == ' '){
            tag++;
        }else{
            info[tag] = info[tag] + *str;
        }
        str++;
    }

    this->ui->listWidget->addItem("[查询请求] " + info[0] + " -> " + info[1] + " " + info[2] + " " + get_seat_name(info[3]));




}

void MainWindow::order(int i,QByteArray &text){
    QByteArray info[6];    //Starting Terminal date flytime seat_class seat_number
    char* str = text.data();
    int tag = 0;
    while(*str != '\0'){
        if(*str == ' '){
            tag++;
        }else{
            info[tag] = info[tag] + *str;
        }
        str++;
    }
    this->ui->listWidget->addItem("[订票请求] " + info[0] + " -> " + info[1] + " " + info[2] + " " + info[3] + get_seat_name(info[4]) + " 座位：" + info[5]);

}

void MainWindow::refund(int i,QByteArray &text){


    this->ui->listWidget->addItem("[退票请求] 订单号：" + text);

}

QString MainWindow::get_seat_name(QByteArray a){
    if(a == "f"){
        return seat_class[0];
    }else if(a == "b"){
        return seat_class[1];
    }else if(a == "e"){
        return seat_class[2];
    }
}




