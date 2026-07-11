import QtQuick 2.15
import QtQuick.Controls

Rectangle {
    id: chatPage
    width: 750
    height: 847
    color: "#F8F9FA"

    // ──► 🎯 🧠 核心黑科技：QML 本地纯 JS 内存临时数据库 ◄──
    property var chatStorage: ({})
    property string currentFriendName: ""

    // ──► 🎯 🛰️ 核心拦截网：全权偷听 C++ 扔过来的所有信号 ◄──
    Connections {
        target: cppBridge

        // 🟢 A. 完美拦截 4 参数切盘信号
        onSig_user_switched: (name, isOnline, lastTime, history) => {
            console.log("🎨 [QML 执行器] 抓取到 C++ 切盘指令 ──► 好友名字:", name)

            // 1. 【备份快照】如果前一任好友名字不为空，把当前留在屏幕上的气泡全打包存入 JS 数据库
            if (chatPage.currentFriendName !== "") {
                var currentHistory = []
                for (var i = 0; i < chatHistoryModel.count; i++) {
                    currentHistory.push(chatHistoryModel.get(i))
                }
                chatPage.chatStorage[chatPage.currentFriendName] = currentHistory
            }

            // 2. 移动当前好友激活指针
            chatPage.currentFriendName = name

            // 3. 动态刷新顶部栏 UI 外观皮肤
            title_lb.text = name
            title_wid.isOnline = isOnline
            online_time_lb.text = isOnline ? "online" : "last online: " + lastTime

            // 4. 洗刷清除上一任残留在屏幕上的气泡
            chatHistoryModel.clear()

            // 5. 【恢复与打捞】优先去 JS 内存库里看看有没有聊过天，如果有就加载出来
            if (chatPage.chatStorage[name] !== undefined) {
                var savedHistory = chatPage.chatStorage[name]
                for (var j = 0; j < savedHistory.length; j++) {
                    chatHistoryModel.append(savedHistory[j])
                }
            } else {
                // 🚀 如果 JS 仓库里没聊过，说明是首次点开，直接把 C++ 刚刚在 Lambda 里塞进来的历史数组给灌进去！
                for (var k = 0; k < history.length; k++) {
                    chatHistoryModel.append(history[k])
                }
            }

            // 6. 滚动条强制置底
            chatListView.positionViewAtEnd()
        }

        // 🟢 B. 接收套接字实时收到的对方新消息
        onSig_new_message_received: (sender, message) => {
            chatHistoryModel.append({ "sender": sender, "message": message })
            chatListView.positionViewAtEnd()
        }
    }

    // 三态 ClickedLabel 组件封装
    component ClickedLabel : Item {
        id: customLabel
        property string normalSrc: ""
        property string hoverSrc: ""
        property string pressSrc: ""
        implicitWidth: 26
        implicitHeight: 26

        Image {
            id: img
            anchors.fill: parent
            source: customLabel.normalSrc
            smooth: true
            scale: mouseArea.pressed ? 0.9 : (mouseArea.containsMouse ? 1.05 : 1.0)
            Behavior on scale { NumberAnimation { duration: 100 } }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
        }

        states: [
            State {
                name: "hover"; when: mouseArea.containsMouse && !mouseArea.pressed
                PropertyChanges { target: img; source: customLabel.hoverSrc }
            },
            State {
                name: "press"; when: mouseArea.pressed
                PropertyChanges { target: img; source: customLabel.pressSrc }
            },
            State {
                name: "normal"; when: !mouseArea.containsMouse && !mouseArea.pressed
                PropertyChanges { target: img; source: customLabel.normalSrc }
            }
        ]
    }

    Column {
        id: chat_data_wid
        anchors.fill: parent
        spacing: 0

        // 顶部状态栏
        Rectangle {
            id: title_wid
            width: parent.width
            height: 65
            color: "#FFFFFF"
            property bool isOnline: true

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#EAEAEA"
            }

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                Label {
                    id: title_lb
                    text: typeof currentChatUserName !== "undefined" ? currentChatUserName : "Select a friend..."
                    font.family: "Microsoft YaHei"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#2C3E50"
                }

                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: title_wid.isOnline ? "#2ECC71" : "#95A5A6"
                    anchors.verticalCenter: parent.verticalCenter
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                Label {
                    id: online_time_lb
                    text: title_wid.isOnline ? "online" : "offline"
                    font.family: "Microsoft YaHei"
                    font.pixelSize: 12
                    color: "#95A5A6"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // 中间聊天视窗 ListView
        Item {
            id: conversation_box
            width: parent.width
            height: parent.height - title_wid.height - tool_wid.height
            clip: true

            ListModel {
                id: chatHistoryModel
            }

            ListView {
                id: chatListView
                anchors.fill: parent
                anchors.margins: 16
                model: chatHistoryModel
                spacing: 12
                Component.onCompleted: chatListView.positionViewAtEnd()

                delegate: Item {
                    width: chatListView.width
                    height: bubbleCard.height + 4
                    readonly property bool isMe: model.sender === "me"

                    Rectangle {
                        id: bubbleCard
                        width: Math.min(msgText.implicitWidth + 24, parent.width * 0.65)
                        height: msgText.implicitHeight + 16
                        radius: 12
                        color: isMe ? "#2CB46E" : "#FFFFFF"
                        anchors.right: isMe ? parent.right : undefined
                        anchors.left: isMe ? undefined : parent.left
                        border.width: isMe ? 0 : 1
                        border.color: "#E5E7E9"

                        Text {
                            id: msgText
                            text: model.message
                            font.family: "Microsoft YaHei"
                            font.pixelSize: 14
                            color: isMe ? "#FFFFFF" : "#2C3E50"
                            anchors.fill: parent
                            anchors.margins: 8
                            verticalAlignment: Text.AlignVCenter
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }

        // 底部工具输入条
        Rectangle {
            id: tool_wid
            width: parent.width
            height: 240
            color: "#FFFFFF"

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: "#EAEAEA"
            }

            Rectangle {
                id: send_wid
                width: parent.width
                height: 45
                color: "transparent"
                anchors.top: parent.top
                anchors.topMargin: 4

                Row {
                    anchors.left: parent.left
                    anchors.leftMargin: 24
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 18

                    ClickedLabel {
                        id: emo_lb
                        normalSrc: "qrc:/rc/chat_picture/smile.png"
                        hoverSrc: "qrc:/rc/chat_picture/smile_hover.png"
                        pressSrc: "qrc:/rc/chat_picture/smile_press.png"
                    }

                    ClickedLabel {
                        id: file_lb
                        normalSrc: "qrc:/rc/chat_picture/filedir.png"
                        hoverSrc: "qrc:/rc/chat_picture/filedir_hover.png"
                        pressSrc: "qrc:/rc/chat_picture/filedir_press.png"
                    }

                    ClickedLabel {
                        id: voice_lb
                        normalSrc: "qrc:/rc/chat_picture/voice.png"
                        hoverSrc: "qrc:/rc/chat_picture/voice_hover.png"
                        pressSrc: "qrc:/rc/chat_picture/voice_press.png"
                    }
                }

                Button {
                    id: send_btn
                    text: "SEND"
                    anchors.right: parent.right
                    anchors.rightMargin: 24
                    anchors.verticalCenter: parent.verticalCenter

                    focusPolicy: Qt.NoFocus
                    hoverEnabled: true

                    background: Rectangle {
                        implicitWidth: 80
                        implicitHeight: 34
                        radius: 17
                        color: send_btn.pressed ? "#1E8449" : (send_btn.hovered ? "#2ECC71" : "#2CB46E")
                        opacity: send_btn.enabled ? 1.0 : 0.5
                        border.color: "transparent"
                        border.width: 0
                        Behavior on color { ColorAnimation { duration: 150 } }
                    }

                    contentItem: Text {
                        text: send_btn.text
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 13
                        font.bold: true
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        if (chatedit.text.trim() === "") return;

                        // 本地长出我的绿色消息气泡
                        chatHistoryModel.append({
                            "sender": "me",
                            "message": chatedit.text
                        });

                        // 🚀 跨语种调用 C++ 的 Q_INVOKABLE 函数，将打字结果送回底层
                        cppBridge.sendMessageFromQml(chatPage.currentFriendName, chatedit.text);

                        chatedit.clear();
                        chatListView.positionViewAtEnd();
                    }
                }
            }

            Rectangle {
                id: chatBoxContainer
                anchors.top: send_wid.bottom
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 16
                anchors.bottomMargin: 20
                color: "#F5F6F8"
                radius: 12
                border.width: 1
                border.color: chatedit.activeFocus ? "#3498DB" : "#E5E7E9"
                Behavior on border.color { ColorAnimation { duration: 150 } }

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 12
                    clip: true

                    TextArea {
                        id: chatedit
                        placeholderText: "Type a message..."
                        placeholderTextColor: "#A0AAB5"
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 15
                        color: "#2C3E50"
                        wrapMode: TextArea.Wrap
                        selectByMouse: true
                        background: null
                    }
                }
            }
        }
    }
}