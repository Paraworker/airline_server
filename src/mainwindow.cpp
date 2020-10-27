#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMutex>

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
            QByteArray addflag = "0%" + airline_name;
            clientSocket[i]->write(addflag);    //发回航空公司名字
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
    else if(text.startsWith("3#")){
        text =text.mid(2);
        return 3;
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
        show_seat(i,array);
    }else if(type == 2){
        order(i,array);
    }else if(type == 3){
        refund(i,array);
    }
}

void MainWindow::air_query(int i,QByteArray &text){     //1%
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

    this->ui->listWidget->addItem("[查询航班信息请求] " + info[0] + " -> " + info[1] + " " + info[2] + " " + get_seat_name(info[3]));

    vector<QByteArray> result;
    QByteArray query_select_sql;        //sql
    int check = select_query(result,query_select_sql);
    if(check == 1){
        QString b = "1%";
        b = b + result.size() + " ";
        for (int j = 0;j < result.size() ; j++) {
            b = b + result[j] + " ";
        }

        clientSocket[i]->write(b.toUtf8().data());
        this->ui->listWidget->addItem("查询航班信息成功，结果已发回");
    }
    else {
        QByteArray b = "5%数据库查询航班信息失败！";
        clientSocket[i]->write(b);
        this->ui->listWidget->addItem("数据库查询航班信息失败，结果已发回");
    }
}

void MainWindow::show_seat(int i,QByteArray &text){     //2%
    QByteArray info[5];       //starting terminal date flytime seat_class
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
    this->ui->listWidget->addItem("[座位信息请求] " + info[0] + " -> " + info[1] + " " + info[2] + " " + info[3] + " " + get_seat_name(info[4]));

    vector<QByteArray> result;
    QByteArray show_seat_select_sql;        //sql
    int check = select_show_seat(result,show_seat_select_sql);
    if(check == 1){
        QString b1 = "2%";
        b1 = b1 + result.size() + " ";
        for (int j = 0;j < result.size() ; j++) {
            b1 = b1 + result[j] + " ";
        }

        clientSocket[i]->write(b1.toUtf8().data());
        this->ui->listWidget->addItem("查询座位信息成功，结果已发回");
    }else{
        QByteArray b2 = "5%数据库查询座位信息失败！";
        clientSocket[i]->write(b2);
        this->ui->listWidget->addItem("数据库查询座位信息失败，结果已发回");
    }
}

void MainWindow::order(int i,QByteArray &text){     //3%
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

    QByteArray order_select_sql;        //sql
    int check = check_exsits(order_select_sql);
    if(check == 1){
        QByteArray order_number;
        int check1 = create_order_number(order_number);
        if(check1 == -1){
            QByteArray msg = "5%生成订单号时数据库操作失败！";
            clientSocket[i]->write(msg);
            this->ui->listWidget->addItem("生成订单号时数据库操作失败，结果已发回");
            return;
        }
        QByteArray order_update_sql;        //sql
        int check2 = update_data(order_update_sql);
        if(check2 == 1){
            QByteArray b1 = "3%订票成功！订单号为：" + order_number;
            clientSocket[i]->write(b1);
            this->ui->listWidget->addItem("订票成功，结果已发回");
        }else{
            QByteArray b2 = "5%选座位时数据库操作失败！";
            clientSocket[i]->write(b2);
            this->ui->listWidget->addItem("选座位时数据库操作失败，结果已发回");
        }
    }else if (check == 0) {
        QByteArray b3 = "3%该座位已被选！";
        clientSocket[i]->write(b3);
        this->ui->listWidget->addItem("座位已被选，结果已发回");
    }else{
        QByteArray b5 = "5%查询座位时数据库操作失败！";
        clientSocket[i]->write(b5);
        this->ui->listWidget->addItem("查询座位时数据库操作失败，结果已发回");
    }

}

void MainWindow::refund(int i,QByteArray &text){        //4%
    this->ui->listWidget->addItem("[退票请求] 订单号：" + text);
    QByteArray refund_select_sql;       //sql
    int check = check_exsits(refund_select_sql);
    if(check == 1){
        QByteArray refund_update_sql;       //sql
        int check2 = update_data(refund_update_sql);
        if(check2 == 1){
            QByteArray b1 = "4%退票成功！";
            clientSocket[i]->write(b1);
            this->ui->listWidget->addItem("退票成功，结果已发回");
        }else{
            QByteArray b2 = "5%退票时数据库操作失败！";
            clientSocket[i]->write(b2);
            this->ui->listWidget->addItem("退票时数据库操作失败，结果已发回");
        }
    }else if(check == 0){
        QByteArray b3 = "4%无此订单！";
        clientSocket[i]->write(b3);
        this->ui->listWidget->addItem("无此订单，结果已发回");
    }else{
        QByteArray b5 = "5%查询订单号时数据库操作失败！";
        clientSocket[i]->write(b5);
        this->ui->listWidget->addItem("查询订单号时数据库操作失败，结果已发回");
    }
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

int MainWindow::select_query(vector<QByteArray> &result,QByteArray sql)
{
    QSqlQuery query(*database);
    QMutex mutex;

    mutex.lock();
    bool isok = query.exec(sql);
    mutex.unlock();

    if(!isok)
    {
        return 0;
    }
    else
    {
        while(query.next())
        {
            QByteArray starting = query.value(0).toByteArray();
            QByteArray terminal = query.value(1).toByteArray();
            QByteArray date = query.value(2).toByteArray();
            QByteArray flytime = query.value(3).toByteArray();
            QByteArray seat_class = query.value(4).toByteArray();
            QByteArray final = starting + " " + terminal + " " + date + " " + flytime + " " + seat_class + " " + airline_name;
            result.push_back(final);
        }
        return 1;
    }
}


int MainWindow::update_data(QByteArray sql){
    QSqlQuery query(*database);
    QMutex mutex;

    mutex.lock();
    int isok = query.exec(sql);
    mutex.unlock();
    if(!isok)
    {
        return -1;
    }
    return 1;
}

int MainWindow::check_exsits(QByteArray sql){
    QSqlQuery query(*database);
    QMutex mutex;

    mutex.lock();
    bool isok = query.exec(sql);
    mutex.unlock();

    if(!isok)
    {
        return -1;
    }
    else
    {
        int tag = 0;
        while(query.next())
        {
            tag++;
        }
        if(tag != 0){
            return 1;
        }else{
            return 0;
        }
    }
}

int MainWindow::select_show_seat(vector<QByteArray> &result,QByteArray sql){
    QSqlQuery query(*database);
    QMutex mutex;

    mutex.lock();
    bool isok = query.exec(sql);
    mutex.unlock();

    if(!isok)
    {
        return 0;
    }
    else
    {
        while(query.next())
        {
            QByteArray seat_number = query.value(0).toByteArray();
            QByteArray final = seat_number + " ";
            result.push_back(final);
        }
        return 1;
    }
}


int MainWindow::create_order_number(QByteArray& number){
    QByteArray sql;
    QSqlQuery query(*database);
    QMutex mutex;

    while(true){
        //number =
        //sql =
        mutex.lock();
        bool isok = query.exec(sql);
        mutex.unlock();
        if(!isok){
            return -1;
        }
        else{
            int tag = 0;
            while(query.next()){
                tag++;
            }
            if(tag != 0){
                continue;
            }else{
                return 1;
            }
        }
    }
}




