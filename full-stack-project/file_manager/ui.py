import tkinter as tk
from tkinter import filedialog
from tkinter import messagebox
from classifier import organise_folder
from config_manager import load_rules,save_rules

class file_organizer_UI:
    def __init__(self,root):
        self.root=root
        self.root.title("Smart File Organizer")

        self.rules=load_rules()

        self.folder_path=tk.StringVar()

        title=tk.Label(root, text="Smart Text Organizer",font=("Arial",16))
        title.pack(pady=10)

        path_entry=tk.Entry(root,textvariable=self.folder_path,width=50)
        path_entry.pack()

        browse=tk.Button(root,text="Browser folder",command=self.browse)
        browse.pack(pady=5)

        organize=tk.Button(root,text="Organize file",command=self.organize)
        organize.pack(pady=10)

        rule_label=tk.Label(root,text="Add new Rules")
        rule_label.pack(pady=10)

        self.folder_name=tk.Entry(root)
        self.folder_name.pack()

        self.keywords=tk.Entry(root)
        self.keywords.pack()

        add_rule=tk.Button(root,text="Add Rules",command=self.add_rule)
        add_rule.pack(pady=5)

    def browse(self):
        folder=filedialog.askdirectory()

        if folder:
            self.folder_path.set(folder)

    def organize(self):
        path=self.folder_path.get()
        if not path:
            messagebox.showwarning("Warning","please select folder")
            return
        organise_folder(path,self.rules)

        messagebox.showinfo("Done","Files Organized")

    def add_rule(self):
        folder=self.folder_name.get()
        keywords=self.keywords.get().split(",")

        if not folder or not keywords:
            messagebox.showwarning("Error","Fill Both Field")
            return 
        self.rules[folder]=keywords
        save_rules(self.rules)
        messagebox.showinfo("Success","Rules Added")



    

