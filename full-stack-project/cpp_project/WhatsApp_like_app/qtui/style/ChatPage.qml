import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: chatPage
    width: 750
    height: 847
    color: "#F8F9FA"

    // 内存自治缓存，用于未拉取 SQL 时的快速切换
    property var chatStorage: ({})
    property string currentFriendName: ""
    property string currentFriendIcon: "" // 补充缺少的外壳资产变量

    // ──► 🛰️ 核心信号连接网（承接 C++ 实体序列化大盲包） ◄──
    Connections {
        target: cppBridge

        // 当 C++ 触发好友切换时，直接抛过来装满历史记录的 history 数组
        onSig_user_switched: (name, isOnline, lastTime, iconPath, history) => {
            chatPage.currentFriendName = name
            chatPage.currentFriendIcon = iconPath
            title_lb.text = name
            title_wid.isOnline = isOnline
            online_time_lb.text = isOnline ? "online" : "last online: " + lastTime

            chatHistoryModel.clear()

            if (history !== undefined && history.length > 0) {
                // 🚀 【防爆破核心安全修护】：遍历并清洗数据包
                for (var i = 0; i < history.length; i++) {
                    var itemData = history[i];

                    // 防御性安全隔离：强行将盲包拆解并本地重新拼装为标准的 JS 字典对象
                    var secureObj = {
                        "sender":  itemData.sender  !== undefined ? itemData.sender  : "other",
                        "type":    itemData.type    !== undefined ? itemData.type    : "text",
                        "content": itemData.content !== undefined ? itemData.content : "",
                        "timeStr": itemData.timeStr !== undefined ? itemData.timeStr : ""
                    };

                    // 满足 Object 格式追加进数据仓库
                    chatHistoryModel.append(secureObj);
                }
            } else {
                // 兜底 Demo 数据（注意格式完全对齐）
                chatHistoryModel.append({ "sender": "other", "type": "text", "content": "Hello! Welcome to QML!", "timeStr": "10:00 AM" })
                chatHistoryModel.append({ "sender": "me",    "type": "text", "content": "Hi there! Let's test bubbles.", "timeStr": "10:01 AM" })
            }
            chatListView.positionViewAtEnd()
        }
    } // 👈 核心修正 1：完美闭合 Connections 拦截网

    // 三态 ClickedLabel 图标组件封装
    component ClickedLabel : Item {
        id: customLabel
        property string normalSrc: ""
        property string hoverSrc: ""
        property string pressSrc: ""
        implicitWidth: 26
        implicitHeight: 26
        Image {
            id: img; anchors.fill: parent; source: customLabel.normalSrc; smooth: true
            scale: mouseArea.pressed ? 0.9 : (mouseArea.containsMouse ? 1.05 : 1.0)
            Behavior on scale { NumberAnimation { duration: 100 } }
        }
        MouseArea { id: mouseArea; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor }
        states: [
            State { name: "hover"; when: mouseArea.containsMouse && !mouseArea.pressed; PropertyChanges { target: img; source: customLabel.hoverSrc } },
            State { name: "press"; when: mouseArea.pressed; PropertyChanges { target: img; source: customLabel.pressSrc } },
            State { name: "normal"; when: !mouseArea.containsMouse && !mouseArea.pressed; PropertyChanges { target: img; source: customLabel.normalSrc } }
        ]
    }

    Column {
        id: chat_data_wid
        anchors.fill: parent
        spacing: 0

        // 顶部状态栏
        Rectangle {
            id: title_wid
            width: parent.width; height: 65; color: "#FFFFFF"
            property bool isOnline: true
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: "#EAEAEA" }
            Row {
                anchors.left: parent.left; anchors.leftMargin: 24; anchors.verticalCenter: parent.verticalCenter; spacing: 12
                Label { id: title_lb; text: "Select a friend..."; font.family: "Microsoft YaHei"; font.pixelSize: 18; font.bold: true; color: "#2C3E50" }
                Rectangle { width: 8; height: 8; radius: 4; color: title_wid.isOnline ? "#2ECC71" : "#95A5A6"; anchors.verticalCenter: parent.verticalCenter }
                Label { id: online_time_lb; text: "offline"; font.family: "Microsoft YaHei"; font.pixelSize: 12; color: "#95A5A6"; anchors.verticalCenter: parent.verticalCenter }
            }
        }

        // 中间聊天核心气泡列表框架
        Item {
            id: conversation_box
            width: parent.width
            height: parent.height - title_wid.height - tool_wid.height
            clip: true

            ListModel { id: chatHistoryModel }

            ListView {
                id: chatListView
                anchors.fill: parent
                anchors.margins: 16
                model: chatHistoryModel
                spacing: 24
                Component.onCompleted: chatListView.positionViewAtEnd()

                delegate: Item {
                    id: chatItemRow
                    width: chatListView.width
                    height: nameLabel.height + 4 + bubbleContainer.height + timeLabel.height + 5

                    readonly property bool isMe: model.sender === "me"
                    readonly property bool isText: model.type === "text"

                    // 👤 A. 头像组件（左右动态镜像对齐）
                    Image {
                        id: avatar
                        width: 42; height: 42
                        anchors.right: chatItemRow.isMe ? parent.right : undefined
                        anchors.left: chatItemRow.isMe ? undefined : parent.left
                        anchors.top: parent.top
                        anchors.topMargin: 20

                        // ──► 🎯 核心修正 2：动态咬合双方的真实头像路径资产 ◄──
                        source: chatItemRow.isMe ? (typeof myOwnAvatar !== 'undefined' ? myOwnAvatar : "qrc:/rc/chat_picture/search.png")
                                                 : (chatPage.currentFriendIcon ? chatPage.currentFriendIcon : "qrc:/rc/chat_picture/search.png")
                        smooth: true
                    }

                    // 📛 B. 用户名组件
                    Label {
                        id: nameLabel
                        // 这里的“恋恋风辰”可以替换为从 C++ 获取的 currentChatUserName
                        text: chatItemRow.isMe ? (typeof currentChatUserName !== 'undefined' ? currentChatUserName : "Me")
                                               : (chatPage.currentFriendName || "User")
                        font.family: "Microsoft YaHei"; font.pixelSize: 11; color: "#7F8C8D"; height: 20
                        anchors.top: parent.top
                        anchors.right: chatItemRow.isMe ? avatar.left : undefined
                        anchors.left: chatItemRow.isMe ? undefined : avatar.right
                        anchors.rightMargin: chatItemRow.isMe ? 12 : 0
                        anchors.leftMargin: chatItemRow.isMe ? 0 : 12
                    }

                    // C. 核心气泡复合框
                    Item {
                        id: bubbleContainer
                        height: chatItemRow.isText ? textContent.implicitHeight + 20 : 160
                        width: chatItemRow.isText ? Math.min(textContent.implicitWidth + 30, parent.width * 0.6) : 200

                        anchors.top: nameLabel.bottom
                        anchors.topMargin: 4
                        anchors.right: chatItemRow.isMe ? avatar.left : undefined
                        anchors.left: chatItemRow.isMe ? undefined : avatar.right
                        anchors.rightMargin: chatItemRow.isMe ? 8 : 0
                        anchors.leftMargin: chatItemRow.isMe ? 0 : 8

                        Rectangle {
                            id: bubbleFrameBg
                            anchors.fill: parent; radius: 8
                            color: chatItemRow.isMe ? "#9EEA6A" : "#FFFFFF"
                            border.width: chatItemRow.isMe ? 0 : 1
                            border.color: "#E5E7E9"

                            Canvas {
                                id: triangleCanvas
                                width: 8; height: 10
                                anchors.top: parent.top; anchors.topMargin: 10
                                anchors.left: chatItemRow.isMe ? parent.right : undefined
                                anchors.right: chatItemRow.isMe ? undefined : parent.left

                                onPaint: {
                                    var ctx = getContext("2d");
                                    ctx.reset();
                                    ctx.fillStyle = chatItemRow.isMe ? "#9EEA6A" : "#FFFFFF";
                                    ctx.beginPath();
                                    if (chatItemRow.isMe) {
                                        ctx.moveTo(0, 0); ctx.lineTo(8, 4); ctx.lineTo(0, 8);
                                    } else {
                                        ctx.moveTo(8, 0); ctx.lineTo(0, 4); ctx.lineTo(8, 8);
                                    }
                                    ctx.closePath(); ctx.fill();

                                    if (!chatItemRow.isMe) {
                                        ctx.strokeStyle = "#E5E7E9"; ctx.lineWidth = 1;
                                        ctx.beginPath(); ctx.moveTo(8, 0); ctx.lineTo(0, 4); ctx.lineTo(8, 8); ctx.stroke();
                                    }
                                }
                            }
                        }

                        // 多行文本渲染
                        Text {
                            id: textContent
                            visible: chatItemRow.isText
                            text: model.content || ""
                            font.family: "Microsoft YaHei"; font.pixelSize: 14; color: "#2C3E50"
                            anchors.fill: parent; anchors.margins: 10
                            wrapMode: Text.Wrap; verticalAlignment: Text.AlignVCenter
                        }

                        // 图片硬件级圆角裁剪
                        Item {
                            id: imageWrapper
                            visible: !chatItemRow.isText
                            anchors.fill: parent
                            anchors.margins: 4

                            layer.enabled: true
                            layer.effect: Rectangle {
                                width: imageWrapper.width; height: imageWrapper.height; radius: 6
                            }

                            Image {
                                id: imageContent
                                source: !chatItemRow.isText ? (model.content || "") : ""
                                fillMode: Image.PreserveAspectCrop
                                anchors.fill: parent
                                smooth: true
                            }
                        }
                    }

                    // D. 时间轴标签
                    Label {
                        id: timeLabel
                        text: model.timeStr || ""
                        font.family: "Microsoft YaHei"; font.pixelSize: 10; color: "#BDC3C7"; height: 15
                        anchors.top: bubbleContainer.bottom
                        anchors.topMargin: 2
                        anchors.right: chatItemRow.isMe ? bubbleContainer.right : undefined
                        anchors.left: chatItemRow.isMe ? undefined : bubbleContainer.left
                    }
                }
            }
        }

        // 底部输入与发送控制台
        Rectangle {
            id: tool_wid
            width: parent.width; height: 240; color: "#FFFFFF"
            Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: "#EAEAEA" }

            // ──► 🎯 核心修正 3：优化了输入框与下方按钮行的上下物理相对关系 ◄──
            // 输入文本编辑区域
            Rectangle {
                id: chatBoxContainer
                anchors.top: parent.top; anchors.bottom: send_wid.top; anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 16; anchors.bottomMargin: 8
                color: "#F5F6F8"; radius: 12; border.width: 1; border.color: chatedit.activeFocus ? "#3498DB" : "#E5E7E9"

                ScrollView {
                    anchors.fill: parent; anchors.margins: 12; clip: true
                    TextArea {
                        id: chatedit
                        placeholderText: "Type a message..."
                        placeholderTextColor: "#A0AAB5"
                        font.family: "Microsoft YaHei"; font.pixelSize: 15; color: "#2C3E50"
                        wrapMode: TextArea.Wrap; selectByMouse: true; background: null
                    }
                }
            }

            // 功能操作与发送按钮栏（置于控制台底端）
            Rectangle {
                id: send_wid
                width: parent.width; height: 50; color: "transparent"
                anchors.bottom: parent.bottom; anchors.bottomMargin: 8

                Row {
                    anchors.left: parent.left; anchors.leftMargin: 24; anchors.verticalCenter: parent.verticalCenter; spacing: 18
                    ClickedLabel { id: emo_lb; normalSrc: "qrc:/rc/chat_picture/smile.png"; hoverSrc: "qrc:/rc/chat_picture/smile_hover.png"; pressSrc: "qrc:/rc/chat_picture/smile_press.png" }
                    ClickedLabel { id: file_lb; normalSrc: "qrc:/rc/chat_picture/filedir.png"; hoverSrc: "qrc:/rc/chat_picture/filedir_hover.png"; pressSrc: "qrc:/rc/chat_picture/filedir_press.png" }
                    ClickedLabel { id: voice_lb; normalSrc: "qrc:/rc/chat_picture/voice.png"; hoverSrc: "qrc:/rc/chat_picture/voice_hover.png"; pressSrc: "qrc:/rc/chat_picture/voice_press.png" }
                }

                Button {
                    id: send_btn
                    text: "SEND"
                    anchors.right: parent.right; anchors.rightMargin: 24; anchors.verticalCenter: parent.verticalCenter
                    focusPolicy: Qt.NoFocus; hoverEnabled: true

                    background: Rectangle {
                        implicitWidth: 80; implicitHeight: 34; radius: 17
                        color: send_btn.pressed ? "#1E8449" : (send_btn.hovered ? "#2ECC71" : "#2CB46E")
                    }

                    contentItem: Text {
                        text: send_btn.text; font.family: "Microsoft YaHei"; font.pixelSize: 13; font.bold: true; color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        if (chatedit.text.trim() === "") return;

                        // 获取实时物理时间戳
                        var currentTime = new Date().toLocaleTimeString(Qt.locale("en_US"), "hh:mm AP");

                        // 1. 本地仓库快速追加
                        chatHistoryModel.append({
                            "sender": "me",
                            "type": "text",
                            "content": chatedit.text,
                            "timeStr": currentTime
                        });

                        // 2. 扔向 C++ 腰部总线进行网络外发
                        if (typeof cppBridge !== 'undefined') {
                            cppBridge.sendMessageFromQml(chatPage.currentFriendName, chatedit.text);
                        }

                        chatedit.clear();
                        chatListView.positionViewAtEnd();
                    }
                }
            }
        }
    }
}