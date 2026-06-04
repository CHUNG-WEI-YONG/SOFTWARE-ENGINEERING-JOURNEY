from tkinter import messagebox
from datetime import datetime

def validate_input(text):
    if not text.strip():
        messagebox.showwarning("Reminder","You enter empty tasks!")
        return False
    return True

def confirm_delete():
    return messagebox.askyesno("Confirm?","Delete your task?")

def validate_time(time):
    try :
        datetime.strptime(time,"%Y-%m-%d %H:%S")
        return True
    except ValueError:
        messagebox.showerror("Error , Wrong time format!")
        return False


