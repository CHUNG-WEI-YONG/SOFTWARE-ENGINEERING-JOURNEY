from django.shortcuts import render ,redirect
from django.contrib.auth.models import User
from todo import models
from todo.models import Todo
from django.contrib.auth import authenticate,login as auth_login,logout
from django.contrib import messages

def signup(request):
    if request.method =='POST':
        fnm=request.POST.get('fnm')
        email=request.POST.get('email')
        password=request.POST.get('pwd')
        print(fnm,email,password)
        my_user=User.objects.create_user(fnm,email,password)
        my_user.save()
        return redirect('/login')

    
    return render(request,'signup.html')

def login(request):
    if request.method =='POST':
        fnm=request.POST.get('fnm')
        pwd=request.POST.get('pwd')
        print(fnm,pwd)
        user=authenticate(request,username=fnm,password=pwd)
        if user is not None:
            auth_login(request , user)
            return redirect('/todolist')
        else:
            messages.error(request,"Wrong Password or Username")
            return render(request,'login.html')


    
    return render(request,'login.html')