
from django.contrib import admin
from django.urls import path
from . import views

urlpatterns = [
    path('admin/', admin.site.urls),
    path('',views.signup),
    path('login/',views.login),
    path('todolist/',views.todolist),
    path('edit_todo/<int:num>',views.edit_todo,name='edit_todo'),
    path('delete/<int:num>',views.delete),
    path('logout/',views.signout),
]
