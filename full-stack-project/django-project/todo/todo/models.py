from django.db import models
from django.contrib.auth.models import User

class Todo(models.Model):
    num=models.AutoField(primary_key=True,auto_created=True)
    title=models.CharField(max_length=25)
    date=models.DateTimeField(auto_now=True)
    user=models.ForeignKey(User, on_delete=models.CASCADE)
    expected_date=models.DateField()

    def __str__(self):
        return self.title