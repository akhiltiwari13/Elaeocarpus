#!/usr/bin/env python3
"""
Contract Master File Viewer

This program loads a pipe-delimited file (such as a contract master file)
and displays its content in a graphical table using Tkinter's Treeview widget.
It provides a menu option to open a file via a file dialog.

The expected file format is as follows (first line as headers):
contract_id|unique_contract_name|primary_contract_name|primary_exchange|...
followed by rows of data.

Tested on Python 3.
"""

import tkinter as tk
from tkinter import ttk
from tkinter import filedialog, messagebox
import csv
import os


def load_data(filename):
    """
    Reads the pipe-delimited file and returns a list of rows.
    Each row is a list of field values.
    """
    data = []
    try:
        with open(filename, newline="", encoding="utf-8") as csvfile:
            reader = csv.reader(csvfile, delimiter="|")
            for row in reader:
                data.append(row)
    except Exception as e:
        messagebox.showerror("Error", f"Could not read file:\n{e}")
    return data


def populate_treeview(data):
    """
    Populates the treeview widget with header and row data.
    """
    # Clear any existing data
    tree.delete(*tree.get_children())
    tree["columns"] = ()
    tree["show"] = "headings"

    if not data:
        return

    headers = data[0]
    tree["columns"] = headers

    # Set up the headings and column properties.
    for header in headers:
        tree.heading(header, text=header)
        tree.column(header, width=100, anchor="center")

    # Insert the rows into the treeview.
    for row in data[1:]:
        tree.insert("", "end", values=row)


def load_file():
    """
    Opens a file dialog for the user to select a contract master file,
    then loads and displays its contents.
    """
    filetypes = (("Text files", "*.txt *.csv"), ("All files", "*.*"))
    filename = filedialog.askopenfilename(
        title="Select Contract Master File", filetypes=filetypes
    )
    if filename:
        data = load_data(filename)
        if data:
            # Set window title to include the file name (without path)
            root.title(f"Contract Master File Viewer - {os.path.basename(filename)}")
            populate_treeview(data)


# Create the main window
root = tk.Tk()
root.title("Contract Master File Viewer")
root.geometry("1200x600")  # Set an initial size

# Create a frame for the treeview and its scrollbars
frame = ttk.Frame(root)
frame.pack(fill="both", expand=True)

# Create the Treeview widget
tree = ttk.Treeview(frame)
tree.pack(side="left", fill="both", expand=True)

# Add vertical scrollbar
vsb = ttk.Scrollbar(frame, orient="vertical", command=tree.yview)
vsb.pack(side="right", fill="y")
tree.configure(yscrollcommand=vsb.set)

# Add horizontal scrollbar
hsb = ttk.Scrollbar(root, orient="horizontal", command=tree.xview)
hsb.pack(side="bottom", fill="x")
tree.configure(xscrollcommand=hsb.set)

# Create a menu bar with a File menu to open files or exit
menubar = tk.Menu(root)
filemenu = tk.Menu(menubar, tearoff=0)
filemenu.add_command(label="Open File", command=load_file)
filemenu.add_separator()
filemenu.add_command(label="Exit", command=root.quit)
menubar.add_cascade(label="File", menu=filemenu)
root.config(menu=menubar)

# Start the main event loop
root.mainloop()
