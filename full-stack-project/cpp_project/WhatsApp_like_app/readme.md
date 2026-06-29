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
