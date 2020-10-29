#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMutex>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    airline_name = "中国南方航空";
    airline_id = "CZ";
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
            list_add("Connect to database failed!");
            return;
        }
        else{
            list_add("Connect to database succeed!");
        }
    }
    bool k = set_socket();
    if(k != true){
        list_add("Set socket failed!");
        return;
    }
    else{
        list_add("Socket listening...");
    }
    this->ui->startButton->setDisabled(true);
    list_add("Running");
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
            list_add("有新的客户端连接请求，但是连接的客户端数量已满");
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
        list_add("A client disconnected");
    });

    list_add("A client connected!      " + QString("[%1]:%2").arg(ip).arg(port));
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
    while(*str != '\0' && tag < 4){
        if(*str == ' '){
            tag++;
        }else{
            info[tag] = info[tag] + *str;
        }
        str++;
    }

    list_add("[查询航班信息请求] " + info[0] + " -> " + info[1] + " " + info[2] + " " + get_seat_name(info[3]));

    vector<QByteArray> result;  //flytime
    QString query_select_sql= "SELECT flight.`flytime` from flight,schedule where flight.`flightnumber` = schedule.`flightnumber` and flight.`starting` = '" + info[0] +  "' and flight.`terminal` = '" + info[1] + "' and schedule.`date` = '" + info[2] + "';";

    int check = sql_select(result,query_select_sql);
    if(check == 1){
        QString b = "1%";
        for (int j = 0;j < result.size() ; j++) {
            b = b + info[0] + " " + info[1] + " " + info[2] + " " + result[j] + " " + info[3] + " " + airline_id + " ";
        }

        clientSocket[i]->write(b.toUtf8().data());
        list_add("查询航班信息成功，结果已发回");
    }
    else if(check == 0){
        QString b = "1%";
        clientSocket[i]->write(b.toUtf8().data());
        list_add("查询航班信息成功（无内容），结果已发回");
    }
    else {
        QByteArray b = "5%数据库查询航班信息失败！";
        clientSocket[i]->write(b);
        list_add("数据库查询航班信息失败，结果已发回");
    }
}

void MainWindow::show_seat(int i,QByteArray &text){     //2%
    QByteArray info[5];       //starting terminal date flytime seat_class
    char* str = text.data();
    int tag = 0;
    while(*str != '\0' && tag < 5){
        if(*str == ' '){
            tag++;
        }else{
            info[tag] = info[tag] + *str;
        }
        str++;
    }
    list_add("[座位信息请求] " + info[0] + " -> " + info[1] + " " + info[2] + " " + info[3] + " " + get_seat_name(info[4]));


    vector<QByteArray> this_scheduleid;
    QString check_sql = "SELECT `scheduleid` FROM flight,schedule where flight.`flightnumber` = schedule.`flightnumber` and flight.`starting` = '" + info[0] +  "' and flight.`terminal` = '" + info[1] + "' and schedule.`date` = '" + info[2] + "' and flight.`flytime` = '" + info[3] + "';";
    int check2 = sql_select(this_scheduleid,check_sql);
    if(check2 == 1){
        vector<QByteArray> result;      //存已经被订的座位
        QString show_seat_select_sql = "SELECT `seatnumber` FROM ticket where `class` = '" + info[4] + "' and `scheduleid`  =  '" + this_scheduleid[0] +"';";
        int check = sql_select(result,show_seat_select_sql);
        if(check == 1){
            QString b1 = "2%";
            for (int j = 0;j < result.size() ; j++) {
                b1 = b1 + result[j] + " ";
            }

            clientSocket[i]->write(b1.toUtf8().data());
            list_add("查询座位信息成功，结果已发回");
        }else if(check == 0){
            QString b1 = "2%";
            clientSocket[i]->write(b1.toUtf8().data());
            list_add("查询座位信息成功（空内容），结果已发回");
        }
        else{
            QByteArray b2 = "5%数据库查询座位信息时失败！";
            clientSocket[i]->write(b2);
            list_add("数据库查询座位信息时失败，结果已发回");
        }
    }else if(check2 == 0){
        QByteArray b3 = "2%并不存在这趟航班！";
        clientSocket[i]->write(b3);
        list_add("并不存在这趟航班，结果已发回");
    }else{
        QByteArray b5 = "5%检查航班是否存在时数据库操作失败！";
        clientSocket[i]->write(b5);
        list_add("检查航班是否存在时数据库操作失败，结果已发回");
    }
}

void MainWindow::order(int i,QByteArray &text){     //3%
    QByteArray info[6];    //Starting Terminal date flytime seat_class seat_number
    char* str = text.data();
    int tag = 0;
    while(*str != '\0' && tag < 6){
        if(*str == ' '){
            tag++;
        }else{
            info[tag] = info[tag] + *str;
        }
        str++;
    }
    list_add("[订票请求] " + info[0] + " -> " + info[1] + " " + info[2] + " " + info[3] + " " + get_seat_name(info[4]) + " 座位：" + info[5]);


    vector<QByteArray> this_scheduleid;
    QString check_sql = "SELECT `scheduleid` FROM flight,schedule where flight.`flightnumber` = schedule.`flightnumber` and flight.`starting` = '" + info[0] +  "' and flight.`terminal` = '" + info[1] + "' and schedule.`date` = '" + info[2] + "' and flight.`flytime` = '" + info[3] + "';";
    int check1 = sql_select(this_scheduleid,check_sql);
    if(check1 == 1){
        QString order_select_sql = "SELECT `scheduleid` FROM ticket where `scheduleid`  = '" + this_scheduleid[0] + "' and `class` = '" + info[4] + "' and `seatnumber` = '" + info[5] + "';";
        int check = sql_select(order_select_sql);
        if(check == 0){
            QByteArray order_number;
            int check1 = create_order_number(order_number);
            if(check1 == -1){
                QByteArray msg = "5%生成订单号时数据库操作失败！";
                clientSocket[i]->write(msg);
                list_add("生成订单号时数据库操作失败，结果已发回");
                return;
            }
            QString order_insert_sql = "INSERT INTO ticket values('" + order_number + "','" + this_scheduleid[0] + "','" + info[4] + "','" +info[5] + "');";
            int check2 = sql_operation(order_insert_sql);
            if(check2 == 1){
                QByteArray b1 = "3%订票成功！订单号为：" + order_number;
                clientSocket[i]->write(b1);
                list_add("订票成功！订单号为：" + order_number + ",结果已发回");
            }else{
                QByteArray b2 = "5%写入订单时数据库操作失败！";
                clientSocket[i]->write(b2);
                list_add("写入订单时数据库操作失败，结果已发回");
            }
        }else if (check == 1) {
            QByteArray b3 = "3%该座位已被选！";
            clientSocket[i]->write(b3);
            list_add("座位已被选，结果已发回");
        }else{
            QByteArray b5 = "5%查询座位时数据库操作失败！";
            clientSocket[i]->write(b5);
            list_add("查询座位时数据库操作失败，结果已发回");
        }

    }else if(check1 == 0){
            QByteArray b3 = "3%并不存在这趟航班！";
            clientSocket[i]->write(b3);
            list_add("并不存在这趟航班，结果已发回");
    }else{
            QByteArray b5 = "5%检查航班是否存在时数据库操作失败！";
            clientSocket[i]->write(b5);
            list_add("检查航班是否存在时数据库操作失败，结果已发回");
    }
}

void MainWindow::refund(int i,QByteArray &text){        //4%
    list_add("[退票请求] 订单号：" + text);
    QString refund_select_sql = "SELECT `ordernumber` FROM  ticket where `ordernumber` = '" + text + "';";
    int check = sql_select(refund_select_sql);
    if(check == 1){
        QByteArray refund_update_sql = "DELETE FROM ticket where ordernumber = '"  + text + "';";
        int check2 = sql_operation(refund_update_sql);
        if(check2 == 1){
            QByteArray b1 = "4%退票成功！";
            clientSocket[i]->write(b1);
            list_add("退票成功，结果已发回");
        }else{
            QByteArray b2 = "5%退票时数据库操作失败！";
            clientSocket[i]->write(b2);
            list_add("退票时数据库操作失败，结果已发回");
        }
    }else if(check == 0){
        QByteArray b3 = "4%无此订单！";
        clientSocket[i]->write(b3);
        list_add("无此订单，结果已发回");
    }else{
        QByteArray b5 = "5%查询订单号时数据库操作失败！";
        clientSocket[i]->write(b5);
        list_add("查询订单号时数据库操作失败，结果已发回");
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

int MainWindow::sql_select(vector<QByteArray> &result,QString sql)
{
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
            QByteArray b = query.value(0).toByteArray();
            result.push_back(b);
            tag++;
        }
        if(tag != 0){
            return 1;
        }
        else{
            return 0;
        }
    }
}


int MainWindow::sql_operation(QString sql){
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

int MainWindow::sql_select(QString sql){
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


int MainWindow::create_order_number(QByteArray& number){
    QString sql;
    QSqlQuery query(*database);
    QMutex mutex;

    while(true){
        number = getRandomNumber();
        sql = "SELECT `ordernumber` FROM  ticket where `ordernumber` = '" + number + "';";
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
            if(tag == 0){
                number = airline_id + number;
                return 1;
            }else{
                continue;
            }
        }
    }
}

QByteArray MainWindow::getRandomNumber()
{
    QByteArray b;
    for(int i=0; i<13; i++)
    {
        b = b + QByteArray::number(QRandomGenerator::global()->bounded(10));
    }
    return b;
}

void MainWindow::list_add(QString s){
    this->ui->listWidget->addItem(s);
    this->ui->listWidget->scrollToBottom();
}




