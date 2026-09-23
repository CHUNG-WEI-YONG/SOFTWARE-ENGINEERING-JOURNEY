import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs

Rectangle {
    id: chatPage
    width: 750
    height: 847
    color: "#F8F9FA"

    property var chatStorage: ({})
    property string currentFriendName: ""
    property string currentFriendIcon: ""
    property bool b_loading_history: false

    // ──► 🎯 新增：待发送文件暂存状态 ◄──
    property string pendingFilePath: ""
    property string pendingFileName: ""

    Connections {
        target: cppBridge

        onSig_user_switched: (name, isOnline, lastTime, iconPath, history) => {
            console.log("🌟 [QML] 切换会话! 目标姓名:", name, "历史记录条数:", (history ? history.length : 0));
            chatPage.currentFriendName = name
            chatPage.currentFriendIcon = iconPath
            title_lb.text = name
            title_wid.isOnline = isOnline
            online_time_lb.text = isOnline ? "online" : "last online: " + lastTime

            // 切换好友时清空草稿文件
            chatPage.pendingFilePath = ""
            chatPage.pendingFileName = ""

            chatHistoryModel.clear()

            if (history !== undefined && history.length > 0) {
                for (var i = 0; i < history.length; i++) {
                    var itemData = history[i];
                    chatHistoryModel.append({
                        "sender":   itemData.sender   !== undefined ? itemData.sender   : "other",
                        "type":     itemData.type     !== undefined ? itemData.type     : "text",
                        "content":  itemData.content  !== undefined ? itemData.content  : "",
                        "fileSize": itemData.fileSize !== undefined ? itemData.fileSize : "",
                        "fileUrl":  itemData.fileUrl  !== undefined ? itemData.fileUrl  : "",
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
                        "fileUrl":  itemData.fileUrl  !== undefined ? itemData.fileUrl  : "",
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

        onSig_new_file_arrive: (friendName, fileName, fileSizeStr, filePath, timeStr, fileToken) => {
            if (friendName === chatPage.currentFriendName) {
                var existed = false;
                for (var i = chatHistoryModel.count - 1; i >= 0; --i) {
                    var it = chatHistoryModel.get(i);
                    if (it.type === "file" && it.sender === "me" && it.content === fileName && it.fileUrl === "") {
                        chatHistoryModel.setProperty(i, "fileUrl", fileToken);
                        if (fileSizeStr !== "") {
                            chatHistoryModel.setProperty(i, "fileSize", fileSizeStr);
                        }
                        existed = true;
                        break;
                    }
                }

                if (!existed) {
                    chatHistoryModel.append({
                        "sender": (filePath !== "") ? "me" : "other",
                        "type": "file",
                        "content": fileName,
                        "fileSize": fileSizeStr,
                        "fileUrl": fileToken,
                        "timeStr": timeStr,
                        "progress": 100,
                        "isDone": true
                    });
                    chatListView.positionViewAtEnd();
                }
            }
        }

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

        onSig_file_upload_complete: (friendName, success, fileUrl) => {
            if (friendName === chatPage.currentFriendName) {
                for (var i = chatHistoryModel.count - 1; i >= 0; --i) {
                    var item = chatHistoryModel.get(i);
                    if (item.type === "file" && item.sender === "me" && !item.isDone) {
                        chatHistoryModel.setProperty(i, "isDone", true);
                        chatHistoryModel.setProperty(i, "progress", success ? 100 : 0);
                        if (success && fileUrl !== "") {
                            chatHistoryModel.setProperty(i, "fileUrl", fileUrl);
                        }
                        break;
                    }
                }
            }
        }
    }

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

    // ──► 🎯 原生文件选择弹窗 ◄──
    FileDialog {
        id: fileDialog
        title: "Please choose a file to send"
        fileMode: FileDialog.OpenFile

        onAccepted: {
            var rawPath = (typeof fileDialog.selectedFile !== 'undefined' && fileDialog.selectedFile.toString() !== "")
                          ? fileDialog.selectedFile.toString()
                          : fileDialog.fileUrl.toString();

            var cleanPath = rawPath.replace(/^(file:\/{2,3})/i, "");
            if (/^\/[a-zA-Z]:/.test(cleanPath)) {
                cleanPath = cleanPath.substring(1);
            }

            if (chatPage.currentFriendName === "") return;

            var fileName = cleanPath.substring(cleanPath.lastIndexOf('/') + 1);
            if (fileName === "") {
                fileName = cleanPath.substring(cleanPath.lastIndexOf('\\') + 1);
            }

            // 🎯 仅暂存选择状态，不在此处发送或挂气泡
            chatPage.pendingFilePath = cleanPath;
            chatPage.pendingFileName = fileName;
            console.log("📎 [QML] 文件已就绪待发送:", fileName, "路径:", cleanPath);
        }
    }

    Column {
        id: chat_data_wid
        anchors.fill: parent
        spacing: 0

        // 顶部好友信息状态栏
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

        // 中间聊天记录列表
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

                    Item {
                        id: bubbleContainer

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
                                        ctx.beginPath(); ctx.moveTo(8, 0); ctx.lineTo(0, 4); ctx.lineTo(8, 8);
                                        ctx.stroke();
                                    }
                                }
                            }
                        }

                        Text {
                            id: textContent
                            visible: chatItemRow.isText
                            text: model.content || ""
                            font.family: "Microsoft YaHei"; font.pixelSize: 14; color: "#2C3E50"
                            anchors.fill: parent; anchors.margins: 10
                            wrapMode: Text.Wrap; verticalAlignment: Text.AlignVCenter
                        }

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
                                        font.family: "Microsoft YaHei"; font.pixelSize: 13; font.bold: true; color: "#2C3E50"
                                        elide: Text.ElideMiddle
                                    }

                                    Row {
                                        width: parent.width
                                        Label {
                                            text: model.fileSize || "File Transfer"
                                            font.family: "Microsoft YaHei"; font.pixelSize: 11; color: "#7F8C8D"
                                        }

                                        Item { width: 10; height: 1 }

                                        Label {
                                            font.family: "Microsoft YaHei"; font.pixelSize: 11
                                            color: model.isDone ? "#27AE60" : "#E67E22"
                                            text: model.isDone ? "✓ Complete" : (model.progress + "%")
                                        }
                                    }
                                }
                            }

                            ProgressBar {
                                id: uploadProgressBar
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 4
                                from: 0; to: 100
                                value: model.progress !== undefined ? model.progress : 0
                                visible: !model.isDone

                                background: Rectangle { radius: 2; color: "#E0E0E0" }
                                contentItem: Item {
                                    Rectangle {
                                        width: uploadProgressBar.visualPosition * parent.width
                                        height: parent.height; radius: 2; color: "#2ECC71"
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

        // ──► 底部操作与输入控制台 ◄──
        Rectangle {
            id: tool_wid
            width: parent.width; height: 250; color: "#FFFFFF"
            Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: "#EAEAEA" }

            // ──► 🎯 新增：文件选中暂存预览条 ◄──
            Rectangle {
                id: pendingFileBar
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.right: parent.right
                anchors.rightMargin: 16
                height: visible ? 38 : 0
                visible: chatPage.pendingFilePath !== ""
                color: "#EBF5FB"
                radius: 6
                border.color: "#AED6F1"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 8

                    Image {
                        source: "qrc:/rc/chat_picture/filedir.png"
                        width: 20; height: 20
                        anchors.verticalCenter: parent.verticalCenter
                        fillMode: Image.PreserveAspectFit
                    }

                    Label {
                        text: chatPage.pendingFileName
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 12
                        font.bold: true
                        color: "#2980B9"
                        elide: Text.ElideMiddle
                        width: parent.width - 60
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    // 取消选择按钮
                    Rectangle {
                        width: 20; height: 20
                        radius: 10
                        color: cancelMouseArea.containsMouse ? "#E74C3C" : "#BDC3C7"
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            text: "✕"
                            color: "white"
                            font.pixelSize: 11
                            font.bold: true
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            id: cancelMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                chatPage.pendingFilePath = "";
                                chatPage.pendingFileName = "";
                            }
                        }
                    }
                }
            }

            // 输入文本编辑区域
            Rectangle {
                id: chatBoxContainer
                anchors.top: pendingFileBar.bottom
                anchors.topMargin: chatPage.pendingFilePath !== "" ? 8 : 12
                anchors.bottom: send_wid.top
                anchors.bottomMargin: 8
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 16
                color: "#F5F6F8"
                radius: 12
                border.width: 1
                border.color: chatedit.activeFocus ? "#3498DB" : "#E5E7E9"

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

                    // ──► 🎯 核心改造：点击发送时一并分发“文件”与“文字” ◄──
                    onClicked: {
                        var textContent = chatedit.text.trim();
                        var hasText = (textContent !== "");
                        var hasFile = (chatPage.pendingFilePath !== "");

                        if (!hasText && !hasFile) return;

                        var currentTime = new Date().toLocaleTimeString(Qt.locale("en_US"), "hh:mm AP");

                        // 1. 如果有待发送文件，触发文件流水线
                        if (hasFile) {
                            chatHistoryModel.append({
                                "sender": "me",
                                "type": "file",
                                "content": chatPage.pendingFileName,
                                "fileSize": "Preparing...",
                                "fileUrl": "",
                                "timeStr": currentTime,
                                "progress": 0,
                                "isDone": false
                            });

                            if (typeof cppBridge !== 'undefined') {
                                cppBridge.uploadFileFromQml(chatPage.currentFriendName, chatPage.pendingFilePath);
                            }

                            // 清理暂存文件状态
                            chatPage.pendingFilePath = "";
                            chatPage.pendingFileName = "";
                        }

                        // 2. 如果输入框有文字，触发普通消息投递
                        if (hasText) {
                            chatHistoryModel.append({
                                "sender": "me",
                                "type": "text",
                                "content": textContent,
                                "timeStr": currentTime,
                                "progress": 100,
                                "isDone": true
                            });

                            if (typeof cppBridge !== 'undefined') {
                                cppBridge.sendMessageFromQml(chatPage.currentFriendName, textContent);
                            }

                            chatedit.clear();
                        }

                        chatListView.positionViewAtEnd();
                    }
                }
            }
        }
    }
}