import tkinter as tk 
from database import Database
import tools
from datetime import datetime

class ToDoApp:
    def __init__(self,root):
        self.db=Database()
        self.root=root
        self.root.title("To Do List")
        self.root.geometry("400x450")

        self.tasks=self.db.load_tasks()

        self.entry=tk.Entry(root,width=30)
        self.entry.pack(pady=10)

        self.time_label=tk.Label(root,text="Enter your expected finished time:").pack();
        self.time_entry=tk.Entry(root,width=30)
        self.time_entry.insert(0,datetime.now().strftime("%Y-%m-%d %H:%M"))
        self.time_entry.pack()


        self.add_btn=tk.Button(root,text="Add tasks",command=self.add_task)
        self.add_btn.pack()

        self.listbox=tk.Listbox(root,width=50,height=15)
        self.listbox.pack(pady=10,padx=10)

        self.del_btn=tk.Button(root,text="Delete tasks",command=self.delete_task)
        self.del_btn.pack()

        self.refresh_listbox()

    def refresh_listbox(self):
        self.listbox.delete(0,tk.END)
        tasks=self.db.load_tasks()
        for row in tasks:
            if len(row)==3:
                content,time,status=row
                display_str=f"{content} : (time:{time}) (status: {status}) "
            else:
                content=row
                display_str=f"{content} : (time:null) "
            self.listbox.insert(tk.END,display_str)
            
        
    def add_task(self):
        content=self.entry.get()
        time=self.time_entry.get()
        if tools.validate_input(content) and tools.validate_time(time):
            self.db.add_task(content,time)
            self.refresh_listbox()
            self.entry.delete(0,tk.END)
    
    def delete_task(self):
        try:
            index=self.listbox.curselection()[0]
            content=self.listbox.get(index);
            selected_task=content.split(":")[0].strip()
            if tools.validate_input(selected_task):
                self.db.delete_task(selected_task)
                self.refresh_listbox();
        except tk.TclError:
            pass


if __name__=="__main__":
    root=tk.Tk()
    app=ToDoApp(root)
    root.mainloop()


                


