import QtQuick 2.15
import QtQuick.Controls

Rectangle {
    id: chatPage
    width: 750
    height: 847
    color: "#F8F9FA"

    // ──► 🎯 👻 1. 抽取封装【ClickedLabel】三态复刻组件（稳固无错版） ◄──
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

        // ──► 🎯 💖 2. 顶部用户状态栏：title_wid（完美支持在线/离线双态驱动） ◄──
        Rectangle {
            id: title_wid
            width: parent.width
            height: 65
            color: "#FFFFFF"

            // ──► 🌟 【这就是你要的状态变量声明方法】 ◄──
            // 真(true)代表在线(绿点+online)，假(false)代表离线(灰点+last online)。未来可由 C++ 动态控制
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
                    // 绑定 C++ 上下文属性，如若未加载则安全兜底显示 "User"
                    text: typeof currentChatUserName !== "undefined" ? currentChatUserName : "User"
                    font.family: "Microsoft YaHei"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#2C3E50"
                }

                // 🟢 动态呼吸圆点（三元表达式：根据 isOnline 自动翻转颜色）
                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: title_wid.isOnline ? "#2ECC71" : "#95A5A6" // 真则清新绿，假则烟灰
                    anchors.verticalCenter: parent.verticalCenter

                    Behavior on color { ColorAnimation { duration: 200 } } // 顺滑过渡
                }

                // 🟢 动态时间标签（三元表达式：根据 isOnline 自动翻转状态文字）
                Label {
                    id: online_time_lb
                    text: title_wid.isOnline ? "online" : "last online: 5 mins ago"
                    font.family: "Microsoft YaHei"
                    font.pixelSize: 12
                    color: "#95A5A6"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // ──► 🎯 💬 3. 中间聊天历史核心气泡对话框 ◄──
        Item {
            id: conversation_box
            width: parent.width
            // 完美自适应高度计算：榨干头部和尾部，剩下的全分给聊天视窗
            height: parent.height - title_wid.height - tool_wid.height
            clip: true // 🛡️ 极其重要：超出边界的内容自动裁剪，防止聊天记录飞出视窗

            // 1. 数据仓库（存储当前会话的历史消息）
            ListModel {
                id: chatHistoryModel
                ListElement { sender: "other"; message: "Hello! Nice to meet you." }
                ListElement { sender: "me"; message: "Hi! How's your project going?" }
            }

            // 2. 渲染视图引擎
            ListView {
                id: chatListView
                anchors.fill: parent
                anchors.margins: 16
                model: chatHistoryModel
                spacing: 12

                // 初始化完成或大小变动时，让列表数据自动滚到最底部
                Component.onCompleted: chatListView.positionViewAtEnd()

                // 3. 气泡皮肤刻画器（Delegate）
                delegate: Item {
                    width: chatListView.width
                    height: bubbleCard.height + 4 // 动态计算单行项的自适应高度

                    // 区分左右：我发送的消息靠右，别人发送的消息靠左
                    readonly property bool isMe: model.sender === "me"

                    Rectangle {
                        id: bubbleCard
                        // 宽度弹性限制：文字少卡片就短，文字长卡片就宽，但最宽不能超过聊天视窗的 65%
                        width: Math.min(msgText.implicitWidth + 24, parent.width * 0.65)
                        height: msgText.implicitHeight + 16
                        radius: 12

                        // 🎨 交互换肤：我方的气泡是清新绿，对方的气泡是纯洁白
                        color: isMe ? "#2CB46E" : "#FFFFFF"

                        // 靠左还是靠右锚定
                        anchors.right: isMe ? parent.right : undefined
                        anchors.left: isMe ? undefined : parent.left

                        // 为对方的白色气泡边缘挂上淡淡的描边增加层级
                        border.width: isMe ? 0 : 1
                        border.color: "#E5E7E9"

                        Text {
                            id: msgText
                            text: model.message
                            font.family: "Microsoft YaHei"
                            font.pixelSize: 14
                            color: isMe ? "#FFFFFF" : "#2C3E50" // 我方气泡配白字，对方配黑字

                            anchors.fill: parent
                            anchors.margins: 8
                            verticalAlignment: Text.AlignVCenter
                            wrapMode: Text.Wrap // 开启核心自动换行防线
                        }
                    }
                }
            }
        }

        // ──► 🎯 💎 4. 底部整个输入与工具总控制台 ◄──
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

            // ──► 🎯 🛠️ 5. 工具栏：send_wid ◄──
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

                // ──► 🎯 🚀 6. 发送按钮：完美消灭直角聚焦框 ◄──
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

                    // ──► 🎯 💫 按下 SEND 时真正的消息入库闭环事件 ◄──
                    onClicked: {
                        // 1. 防御拦截：如果输入框全为空格或者没打字，直接摆摆手拦截
                        if (chatedit.text.trim() === "") {
                            return;
                        }

                        console.log("🚀 [QML 渲染总线] 发射新消息到面板:", chatedit.text);

                        // 2. 向 ListModel 数据仓库尾部追加一条我方发送的属性记录
                        chatHistoryModel.append({
                            "sender": "me",
                            "message": chatedit.text
                        });

                        // 3. 顺滑清空 TextArea 打字机内容
                        chatedit.clear();

                        // 4. 物理强制列表滚动置底，露出最新追加的那个绿色气泡
                        chatListView.positionViewAtEnd();
                    }
                }
            }

            // ──► 🎯 ✍️ 7. 输入框外壳盒 ◄──
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