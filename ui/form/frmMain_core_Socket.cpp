#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"

#include "common/tableview_controller.h"
#include "ExcelReader.h"

#include <QList>
#include <QScriptEngine>

void frmMain::startSocketServer()
{
    bool isConnect = false;
    if (g_config->getWorkConfig().isSocketServer){
        if (g_config->getWorkConfig().socketServerType == SocketServerTypeEnum::web){
            // 设置和启动WebSocket服务器
            if (!this->m_webSocketServer->listen(QHostAddress::Any, g_config->getWorkConfig().socketPort)) {
                qDebug() << "web socket server could not start: " << this->m_webSocketServer->errorString();
            }else {
                qDebug() << "web socket server started on " << g_config->getWorkConfig().socketPort;
                connect(this->m_webSocketServer, &QWebSocketServer::newConnection, this, &frmMain::handlerWebSocketNewConnection);

                QIcon nullIcon;
                this->m_customStatusBar->addIcon(nullIcon, this->StatusBar_IconName_Socket, "", nullptr);
            }
        }else{
            if (!this->m_tcpServer->listen(QHostAddress::Any, g_config->getWorkConfig().socketPort)) {
                // 处理错误，例如显示一个消息框或写入日志
                qDebug() << "tcp server could not start: " << m_tcpServer->errorString();
            } else {
                qDebug() << "tcp server started on " << g_config->getWorkConfig().socketPort;
                connect(m_tcpServer, &QTcpServer::newConnection, this, &frmMain::handleSocketNewConnection);
            }
        }
    }

}

void frmMain::handleSocketNewConnection()
{
    QTcpSocket *clientSocket = m_tcpServer->nextPendingConnection();
    this->m_tcpClients.append(clientSocket);
    connect(clientSocket, &QTcpSocket::readyRead, this, &frmMain::onTcpReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
}

void frmMain::onTcpReadyRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (clientSocket) {
        QByteArray data = clientSocket->readAll();
        qDebug() << "Received data:" << data; // 打印数据

        //
        this->parseSocketClientData(data, "");

        // 根据需要发送响应给客户端
        clientSocket->write("ok");
    }
}

QString cleanIp(const QString& oIp) {
    QString ip = oIp;
    if (ip.startsWith("::ffff:")) {
        return ip.mid(7); // 移除 IPv4-mapped IPv6 地址前缀
    }
    return ip.remove('[').remove(']'); // 移除 IPv6 地址的方括号
}

void frmMain::handlerWebSocketNewConnection()
{
    if (!m_webSocketServer->hasPendingConnections()) {
        qWarning() << "No pending connections to handle.";
        return;
    }

    QWebSocket *clientSocket = m_webSocketServer->nextPendingConnection();
    if (clientSocket) {
        QString clientIp = cleanIp(clientSocket->peerAddress().toString());
        qDebug() << "New WebSocket client connected:" << clientIp;
        m_webSocketClients[clientIp] = clientSocket;  // 使用映射存储客户端，以IP为键

        //
        QStringList messages =  m_webSocketClients.keys();
        if (g_config->getMeasuringStationConfig().isOpen){
            for (QString& key : messages){
                auto entryIndex = this->getScanEntryIndex(key);
                key += " 进板口" + QString::number(entryIndex+1);
            }
        }
        this->m_customStatusBar->
                setTooltip(this->StatusBar_IconName_Socket, messages.join("\n")); // socket 连接客户端列表

        // 文本类型数据
        connect(clientSocket, &QWebSocket::textMessageReceived, this, [this, clientSocket](const QString &message) {
             QString clientIp = cleanIp(clientSocket->peerAddress().toString());
            qDebug() << "Client:" << clientIp << ", Received data:" << message; // 打印数据

            // 现在您可以知道是哪个客户端发送的消息
            bool isSuccess = this->parseSocketClientData(message, clientIp);
            if (!isSuccess) {
                qWarning() << "json parse error from client:" << clientIp;
            }
        });

        // 二进制类型的数据
        connect(clientSocket, &QWebSocket::binaryMessageReceived, this, [this, clientSocket](const QByteArray &binaryMessage) {
             QString clientIp = cleanIp(clientSocket->peerAddress().toString());
            qDebug() << "Client:" << clientIp << ", Received data:" << binaryMessage.size(); // 打印数据

            // 现在您可以知道是哪个客户端发送的消息
            bool isSuccess = this->parseSocketClientData(binaryMessage, clientIp);
            if (!isSuccess) {
                qWarning() << "json parse error from client:" << clientIp;
            }
        });

        connect(clientSocket, &QWebSocket::disconnected, this, [this, clientIp, clientSocket]() {
            qDebug() << "Client disconnected:" << clientIp;
            clientSocket->deleteLater();
            m_webSocketClients.remove(clientIp);  // 断开连接时移除客户端

            //
            QStringList messages =  m_webSocketClients.keys();
            if (g_config->getMeasuringStationConfig().isOpen){
                for (QString& key : messages){
                    auto entryIndex = this->getScanEntryIndex(key);
                    key += " 进板口" + QString::number(entryIndex+1);
                }
            }
            this->m_customStatusBar->
                    setTooltip(this->StatusBar_IconName_Socket, messages.join("\n")); // socket 连接客户端列表
        });
    } else {
        qWarning() << "Failed to retrieve client socket from the server.";
    }
}

// 处理板件条码
void frmMain::handler4PanelBarccode()
{
    QString barcode = this->ui->txtPanelBarcode->text();
    if (barcode.isEmpty()){
        return;
    }

    auto [length, width, height] = this->handler4Barccode(barcode);
            if (length > 0 && width > 0 && height > 0) {
        int id = 1;
        if (!this->m_panels.isEmpty()){
            auto maxIt = std::max_element(this->m_panels.begin(), this->m_panels.end(),
                                          [](const Panel &a, const Panel &b) { return a.id < b.id; });
            id = maxIt->id + 1;
        }
        Panel panel(id, length, width, height, "", "");
        this->m_panels.append(panel);

        // 清空input
        this->ui->txtPanelBarcode->clear();
    }
}

bool frmMain::parseSocketClientData(const QByteArray &binaryMessage, QString clientIp)
{
    QTextCodec *codec;
    QString decodedString;
    QStringList encodings = {"UTF-8", "GBK", "GB18030", "ISO-8859-1", "Big5"};
    foreach (const QString &encoding, encodings) {
        codec = QTextCodec::codecForName(encoding.toUtf8());
        decodedString = codec->toUnicode(binaryMessage);

        // 解析JSON数据
        QJsonParseError parseError;
        QJsonDocument document = QJsonDocument::fromJson(decodedString.toUtf8(), &parseError);

        // 检查JSON数据是否正确解析
        if (parseError.error != QJsonParseError::NoError) {
            // 解析错误，处理错误情况
            qDebug() << "JSON parse error:" << parseError.errorString();
            continue; // 使用下一种编码进行解析
        }

        if (!document.isObject()) {
            qWarning() << "JSON is not an object.";
            break;
        }

        bool isSuccess = this->parseSocketClientData(decodedString, clientIp);
        if (!isSuccess){
            qWarning() << "json parse error!";
        }
        return  isSuccess;
    }
    return false;
}

bool frmMain::parseSocketClientData(QString message, QString clientIp)
{
    //1. 解析包裹数据
    // 打印数据
    qDebug() << "Received data: " << message;

    // 解析JSON数据
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(message.toUtf8(), &parseError);

    // 检查JSON数据是否正确解析
    if (parseError.error != QJsonParseError::NoError) {
        // 解析错误，处理错误情况
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    if (!document.isObject()) {
        qWarning() << "JSON is not an object.";
        return false;
    }

    // 如果JSON数据是一个对象
    QJsonObject jsonObj = document.object();
    QString packNo = jsonObj.value("ID").toString();
    QString customerName = jsonObj.value("CustomerName").toString();
    QString orderNo = jsonObj.value("OrderNo").toString();

    // 处理Panel列表字段
    QList<Panel> panels;
    QJsonArray panelsArray = jsonObj.value("Boards").toArray();
    for (const QJsonValue &panelVal : qAsConst(panelsArray)) {
        if (panelVal.isObject()) {
            QJsonObject panelObj = panelVal.toObject();
            Panel panel;
            panel.externalId = panelObj.value("ID").toString();

            int length = panelObj.value("Length").toInt();
            int width =  panelObj.value("Width").toInt();
            panel.length = std::max(length, width);
            panel.width = std::min(length, width);

            panel.height = panelObj.value("Height").toInt();
            panel.name = panelObj.value("Name").toString();
            panel.remark = panelObj.value("Remark").toString();
            panel.location = panelObj.value("Location").toString();
            panel.sculpt = panelObj.value("Sculpt").toString();
            panels.append(panel); // 添加到列表
        }
    }

    // 计算包裹
    PackageAO pack = this->m_algorithm->createLayers(panels);
    pack.no = packNo;
    pack.customerName = customerName;
    pack.orderNo = orderNo;    
    PackageDto packDto(pack, PackageDto::Status_Step1_Calculated); // 传输对象
    packDto.originIp = clientIp;

    // 创建包裹数据
    int newPackId = this->m_packBll->insertByPackStruct(packDto);
    this->initForm_PackDataBinding(); // 必须运行一下，否则后面再 运行update status的时候会报错

    // 赋值
    pack.id = packDto.id = newPackId;

    //2. 流转
    this->runFlow(packDto);

    // 成功处理
    return true;
}

