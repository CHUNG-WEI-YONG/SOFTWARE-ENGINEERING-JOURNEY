## This is my journey in learning a project app
I use to learn qt project and also http request

## DAY1
I build up a qt project with ui in login dialog

## DAY 2
adding a signin dialog with signal button connected.
I connect a button "Sign In " with signal connected from clicked button to a the function call in signin dialog , then the signin dialog fnction send a signal to mainwindow which call a function that change the window shown

## DAY 3
adding a basic style and error message to the sign in page and also adding verification of email address

## DAY4
adding a http request manager for sign in pae by using signal and slots with jsonobject , this help to build up the email verify function

## DAY5
Building up a Logic system that helps to register for post and get method link , then handle post and get 
The handle get function help to search at _get_handles based on the requested string and also pass in the function inside the _get_handles

## DAY6 
Using config.ini to set up the port for both gateserver and qtui with class config to manage the port and ip address to make the code easier to be read

## DAY 7
Adding a new VarifyServer by using nodejs and javascipt to send email to verification to the user entered email 
in qtui so by now three server with qtui accept email and send rquest to gateserver port then the gateserver initialise 
a server class with logic system to handdle the post request then send it to varifyserver with what message.proto set up then the varifyserver continue 
to read and send the email to the sign in email

## DAY 8
Using a connection pool to manage httpconnection class and also grpc connection to handle the high concurrency request to allow gateserver and webserver to handle more request at the same time 
and also using a thread pool to manage the thread for each connection to avoid blocking the main thread

## DAY 9
Adding the small function for user to see the password when entering in line edit for password and confirm password by changing the state with qss file design

## DAY 10
By using a timer in the sign in dialog in qt , after successfully sign in the account , user will directed to a page showing sign in successful and return to log in page in 5 s,
this using the slot and signal function in qt.

## DAY 11 
Adding ui page for reset password function and adding a new route at LogicSystem to manage the logic then a new function in class MysqlDao and Mysqlmgr to reset the password

## DAY 12
Adding logic in log in ui of qtui to allow user login function that send a Httphandlepost to gateserver , the function of handle user_login in logicsystem done , mgr and dao of sql is
added with a checkpassword function and return the query of userinfo with name,token and id.

## DAY 13 
Adding the StatusServer that handle the connection pool with struct server that record port and ip and the connection count , it return the least connection count to the gateServer while recording the id and token

## DAY 14
Writing the tcp connection handle at ChatServer that use four functions , readhead,readlen,readbody,and readall, readlen is the function which read all the required length of byte then use the lambda function pass in to processs the data.
Readhead call readall for the first four byte and get the id and length then call readbody,readbody read the next body and if not until the length it will continue reading. After reading all , it will save in a class Csession ,  a Csession is a class that manage by server for each user , it save , send and receive user data

## DAY 15
Adding login logic in the gateserver that allow it to return the userinfo to the frontend qtui , then also add on tcpmanager at qtui to manage the tcp connection with the chatserver.

## WHOLE PROCESS UNTIL NOW
The qtui send a login to the gateserver , then gateserver use sql to find the record of the user , if has , then gateserver use grpc to call getChatServer at the statusServer , statusServer get and save the id and generate a token then save the id and token in an unordered map , then the statusServer return it to gateserver through grpc , then gateserver use http to send to qtui. Qtui used the info return to connect to the server and return success when connected , but without changing page.

## DAY 16
Write for the qt ui frontend for the chatserver , the ui has three parts .with tool widget , chat user widget with a small component widget for each user info , then use a qml file to do for the desgin of user chatpage.

