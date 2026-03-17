import tkinter as tk
from ui import file_organizer_UI

def main():
    print("Programme running")
    root=tk.Tk()
    app =file_organizer_UI(root)
    root.mainloop()

if __name__=="__main__":
    main()