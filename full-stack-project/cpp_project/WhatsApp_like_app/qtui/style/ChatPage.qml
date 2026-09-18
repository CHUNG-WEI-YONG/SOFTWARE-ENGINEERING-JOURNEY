import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3

Rectangle {
    id: chatPage
    width: 750
    height: 847
    color: "#F8F9FA"

    // 内存自治缓存，用于未拉取 SQL 时的快速切换
    property var chatStorage: ({})
    property string currentFriendName: ""
    property string currentFriendIcon: ""

    // ──► 🎯 触顶打捞状态锁，防止滚轮狂滚导致重复请求爆破 ◄──
    property bool b_loading_history: false

    // ──► 🛰️ 核心信号连接网（承接 C++ 实体序列化数据） ◄──
    Connections {
        target: cppBridge

        // 当 C++ 触发好友切换时，加载历史记录
        onSig_user_switched: (name, isOnline, lastTime, iconPath, history) => {
            console.log("🌟 [QML] 切换会话! 目标姓名:", name, "历史记录条数:", (history ? history.length : 0));
            chatPage.currentFriendName = name
            chatPage.currentFriendIcon = iconPath
            title_lb.text = name
            title_wid.isOnline = isOnline
            online_time_lb.text = isOnline ? "online" : "last online: " + lastTime

            chatHistoryModel.clear()

            if (history !== undefined && history.length > 0) {
                for (var i = 0; i < history.length; i++) {
                    var itemData = history[i];
                    chatHistoryModel.append({
                        "sender":   itemData.sender   !== undefined ? itemData.sender   : "other",
                        "type":     itemData.type     !== undefined ? itemData.type     : "text",
                        "content":  itemData.content  !== undefined ? itemData.content  : "",
                        "fileSize": itemData.fileSize !== undefined ? itemData.fileSize : "",
                        "timeStr":  itemData.timeStr  !== undefined ? itemData.timeStr  : "",
                        "progress": 100,
                        "isDone":   true
                    });
                }
            } else {
                chatHistoryModel.append({ "sender": "other", "type": "text", "content": "Hello! Welcome to chat.", "timeStr": "10:00 AM", "progress": 100, "isDone": true })
            }

            chatPage.b_loading_history = false
            chatListView.positionViewAtEnd()
        }

        // 收到好友普通文本消息
        onSig_new_message_received: (sender, message, timeStr) => {
            if (sender === chatPage.currentFriendName) {
                chatHistoryModel.append({
                    "sender": "other",
                    "type": "text",
                    "content": message,
                    "timeStr": timeStr || new Date().toLocaleTimeString(Qt.locale("en_US"), "hh:mm AP"),
                    "progress": 100,
                    "isDone": true
                });
                chatListView.positionViewAtEnd();
            }
        }

        // 下拉触顶加载旧历史记录
        onSig_append_history_batch: (olderHistory) => {
            if (olderHistory !== undefined && olderHistory.length > 0) {
                var oldFirstIndex = chatListView.indexAt(chatListView.contentX, chatListView.contentY);

                for (var i = olderHistory.length - 1; i >= 0; i--) {
                    var itemData = olderHistory[i];
                    chatHistoryModel.insert(0, {
                        "sender":   itemData.sender   !== undefined ? itemData.sender   : "other",
                        "type":     itemData.type     !== undefined ? itemData.type     : "text",
                        "content":  itemData.content  !== undefined ? itemData.content  : "",
                        "fileSize": itemData.fileSize !== undefined ? itemData.fileSize : "",
                        "timeStr":  itemData.timeStr  !== undefined ? itemData.timeStr  : "",
                        "progress": 100,
                        "isDone":   true
                    });
                }

                if (oldFirstIndex >= 0) {
                    chatListView.positionViewAtIndex(oldFirstIndex + olderHistory.length, ListView.Beginning);
                }
            }
            chatPage.b_loading_history = false;
        }

        // ──► 📁 1. 新文件到达（本人发起上传或对端传来文件） ◄──
        onSig_new_file_arrive: (friendName, fileName, fileSizeStr, filePath, timeStr, fileToken) => {
            if (friendName === chatPage.currentFriendName) {
                chatHistoryModel.append({
                    "sender": (filePath !== "") ? "me" : "other",
                    "type": "file",
                    "content": fileName,
                    "fileSize": fileSizeStr,
                    "fileUrl": fileToken, // 👈 存入模型供点击事件使用！
                    "timeStr": timeStr,
                    "progress": 100,
                    "isDone": true
                });
                chatListView.positionViewAtEnd();
            }
        }

        // ──► 📁 2. 刷新文件上传百分比 ◄──
        onSig_file_upload_progress: (friendName, percent) => {
            if (friendName === chatPage.currentFriendName) {
                for (var i = chatHistoryModel.count - 1; i >= 0; --i) {
                    var item = chatHistoryModel.get(i);
                    if (item.type === "file" && item.sender === "me" && !item.isDone) {
                        chatHistoryModel.setProperty(i, "progress", percent);
                        break;
                    }
                }
            }
        }

        // ──► 📁 3. 上传完毕 ◄──
        onSig_file_upload_complete: (friendName, success, fileUrl) => {
            if (friendName === chatPage.currentFriendName) {
                for (var i = chatHistoryModel.count - 1; i >= 0; --i) {
                    var item = chatHistoryModel.get(i);
                    if (item.type === "file" && item.sender === "me" && !item.isDone) {
                        chatHistoryModel.setProperty(i, "isDone", true);
                        chatHistoryModel.setProperty(i, "progress", success ? 100 : 0);
                        break;
                    }
                }
            }
        }
    }

    // 三态 ClickedLabel 图标组件封装
    component ClickedLabel : Item {
        id: customLabel
        property string normalSrc: ""
        property string hoverSrc: ""
        property string pressSrc: ""
        implicitWidth: 26
        implicitHeight: 26

        signal clicked()

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
            onClicked: customLabel.clicked()
        }
        states: [
            State { name: "hover"; when: mouseArea.containsMouse && !mouseArea.pressed; PropertyChanges { target: img; source: customLabel.hoverSrc } },
            State { name: "press"; when: mouseArea.pressed; PropertyChanges { target: img; source: customLabel.pressSrc } },
            State { name: "normal"; when: !mouseArea.containsMouse && !mouseArea.pressed; PropertyChanges { target: img; source: customLabel.normalSrc } }
        ]
    }

    // ──► 🎯 文件选择原生弹窗组件 ◄──
    FileDialog {
        id: fileDialog
        title: "Please choose a file to send"
        folder: shortcuts.home
        selectMultiple: false

        onAccepted: {
            var rawPath = fileDialog.fileUrl.toString();
            var cleanPath = rawPath.replace(/^(file:\/{3})/,"");
            if (cleanPath === rawPath) {
                cleanPath = rawPath.replace(/^(file:\/{2})/,"");
            }

            console.log("📁 [QML] 用户选择了文件:", cleanPath);

            if (chatPage.currentFriendName === "") {
                console.log("⚠️ 未选中好友，终止上传");
                return;
            }

            if (typeof cppBridge !== 'undefined') {
                cppBridge.uploadFileFromQml(chatPage.currentFriendName, cleanPath);
            }
        }

        onRejected: {
            console.log("❌ 用户取消了文件选择");
        }
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
                clip: true

                onContentYChanged: {
                    if (chatListView.contentY < -30 && !chatPage.b_loading_history) {
                        if (chatPage.currentFriendName === "") return;
                        chatPage.b_loading_history = true;
                        console.log("📥 [QML 触顶] 开始拉取好友历史:", chatPage.currentFriendName);
                        if (typeof cppBridge !== 'undefined') {
                            cppBridge.loadMoreHistoryFromQml(chatPage.currentFriendName);
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    id: vScrollBar
                    width: 8
                    policy: ScrollBar.AsNeeded
                    contentItem: Rectangle {
                        implicitWidth: 8; radius: 4
                        color: vScrollBar.pressed ? "#7F8C8D" : (vScrollBar.hovered ? "#95A5A6" : "#BDC3C7")
                    }
                }

                delegate: Item {
                    id: chatItemRow
                    width: chatListView.width
                    height: nameLabel.height + 4 + bubbleContainer.height + timeLabel.height + 5

                    readonly property bool isMe: model.sender === "me"
                    readonly property bool isText: model.type === "text"
                    readonly property bool isFile: model.type === "file"

                    // 👤 A. 头像组件
                    Image {
                        id: avatar
                        width: 42; height: 42
                        anchors.right: chatItemRow.isMe ? parent.right : undefined
                        anchors.left: chatItemRow.isMe ? undefined : parent.left
                        anchors.top: parent.top
                        anchors.topMargin: 20

                        source: chatItemRow.isMe ? (typeof myOwnAvatar !== 'undefined' ? myOwnAvatar : "qrc:/rc/chat_picture/search.png")
                                                 : (chatPage.currentFriendIcon ? chatPage.currentFriendIcon : "qrc:/rc/chat_picture/search.png")
                        smooth: true
                    }

                    // 📛 B. 用户名组件
                    Label {
                        id: nameLabel
                        text: chatItemRow.isMe ? (typeof currentChatUserName !== 'undefined' ? currentChatUserName : "Me")
                                               : (chatPage.currentFriendName || "User")
                        font.family: "Microsoft YaHei"; font.pixelSize: 11; color: "#7F8C8D"; height: 20
                        anchors.top: parent.top
                        anchors.right: chatItemRow.isMe ? avatar.left : undefined
                        anchors.left: chatItemRow.isMe ? undefined : avatar.right
                        anchors.rightMargin: chatItemRow.isMe ? 12 : 0
                        anchors.leftMargin: chatItemRow.isMe ? 0 : 12
                    }

                    // ──► 💬 C. 核心气泡复合框 ◄──
                    Item {
                        id: bubbleContainer

                        // 尺寸动态调整：文件卡片固定高 80 宽 260
                        height: chatItemRow.isFile ? 80 :
                                (chatItemRow.isText ? textContent.implicitHeight + 20 : 160)

                        width: chatItemRow.isFile ? 260 :
                               (chatItemRow.isText ? Math.min(textContent.implicitWidth + 30, parent.width * 0.6) : 200)

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
                                        ctx.beginPath(); ctx.moveTo(8, 0); ctx.lineTo(0, 4); ctx.lineTo(8, 8); stroke();
                                    }
                                }
                            }
                        }

                        // 1. 文本内容显示
                        Text {
                            id: textContent
                            visible: chatItemRow.isText
                            text: model.content || ""
                            font.family: "Microsoft YaHei"; font.pixelSize: 14; color: "#2C3E50"
                            anchors.fill: parent; anchors.margins: 10
                            wrapMode: Text.Wrap; verticalAlignment: Text.AlignVCenter
                        }

                        // 2. 文件卡片显示
                        Item {
                            id: fileCardWrapper
                            visible: chatItemRow.isFile
                            anchors.fill: parent
                            anchors.margins: 10

                            Row {
                                id: fileMainRow
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.right: parent.right
                                spacing: 10

                                Image {
                                    id: fileIcon
                                    width: 40; height: 40
                                    source: "qrc:/rc/chat_picture/filedir.png"
                                    fillMode: Image.PreserveAspectFit
                                }

                                Column {
                                    width: parent.width - fileIcon.width - 10
                                    spacing: 3

                                    Label {
                                        width: parent.width
                                        text: model.content || "File"
                                        font.family: "Microsoft YaHei"
                                        font.pixelSize: 13
                                        font.bold: true
                                        color: "#2C3E50"
                                        elide: Text.ElideMiddle
                                    }

                                    Row {
                                        width: parent.width
                                        Label {
                                            text: model.fileSize || "File Transfer"
                                            font.family: "Microsoft YaHei"
                                            font.pixelSize: 11
                                            color: "#7F8C8D"
                                        }

                                        Item { width: 10; height: 1 }

                                        // 进度与状态文本
                                        Label {
                                            font.family: "Microsoft YaHei"
                                            font.pixelSize: 11
                                            color: model.isDone ? "#27AE60" : "#E67E22"
                                            text: model.isDone ? "✓ Complete" : (model.progress + "%")
                                        }
                                    }
                                }
                            }

                            // 传输中的细进度条
                            ProgressBar {
                                id: uploadProgressBar
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 4
                                from: 0
                                to: 100
                                value: model.progress !== undefined ? model.progress : 0
                                visible: !model.isDone

                                background: Rectangle {
                                    radius: 2
                                    color: "#E0E0E0"
                                }
                                contentItem: Item {
                                    Rectangle {
                                        width: uploadProgressBar.visualPosition * parent.width
                                        height: parent.height
                                        radius: 2
                                        color: "#2ECC71"
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (model.fileUrl && typeof cppBridge !== 'undefined') {
                                        cppBridge.downloadFileFromQml(model.fileUrl, model.content);
                                    }
                                }
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

            // 功能操作与发送按钮栏
            Rectangle {
                id: send_wid
                width: parent.width; height: 50; color: "transparent"
                anchors.bottom: parent.bottom; anchors.bottomMargin: 8

                Row {
                    anchors.left: parent.left; anchors.leftMargin: 24; anchors.verticalCenter: parent.verticalCenter; spacing: 18
                    ClickedLabel { id: emo_lb; normalSrc: "qrc:/rc/chat_picture/smile.png"; hoverSrc: "qrc:/rc/chat_picture/smile_hover.png"; pressSrc: "qrc:/rc/chat_picture/smile_press.png" }
                    ClickedLabel {
                        id: file_lb
                        normalSrc: "qrc:/rc/chat_picture/filedir.png"
                        hoverSrc: "qrc:/rc/chat_picture/filedir_hover.png"
                        pressSrc: "qrc:/rc/chat_picture/filedir_press.png"
                        onClicked: {
                            if (chatPage.currentFriendName === "") {
                                console.log("⚠️ 请先在左侧选择要聊天的好友再发送文件！");
                                return;
                            }
                            fileDialog.open();
                        }
                    }
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

                        var currentTime = new Date().toLocaleTimeString(Qt.locale("en_US"), "hh:mm AP");

                        chatHistoryModel.append({
                            "sender": "me",
                            "type": "text",
                            "content": chatedit.text,
                            "timeStr": currentTime,
                            "progress": 100,
                            "isDone": true
                        });

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