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

def todolist(request):
    if(request.method=='POST'):
        tasks=request.POST.get('task')
        time=request.POST.get('expected_date')
        print(tasks)
        obj=models.Todo(title=tasks,expected_date=time,user=request.user)
        obj.save()
        res=models.Todo.objects.filter(user=request.user).order_by('-date')
        return redirect('/todolist/',{'res':res})
    res=models.Todo.objects.filter(user=request.user).order_by('-date')
    return render(request,'todo.html',{'res':res})

def edit_todo(request,num):
    if(request.method=='POST'):
        tasks=request.POST.get('task')
        time=request.POST.get('expected_date')
        print(tasks)
        obj=models.Todo.objects.get(num=num)
        obj.title=tasks
        obj.expected_date=time;
        obj.save()
        return redirect('/todolist/')
    obj=models.Todo.objects.get(num=num)
    res=models.Todo.objects.filter(user=request.user).order_by('-date')
    return render(request,'edit_todo.html',{'obj':obj,'res':res})
