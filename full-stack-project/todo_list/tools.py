from tkinter import messagebox

def validate_input(text):
    if not text.strip():
        messagebox.showwarning("Reminder","You enter empty tasks!")
        return False
    return True

def confirm_delete():
    return messagebox.askyesno("Confirm?","Delete your task?")
