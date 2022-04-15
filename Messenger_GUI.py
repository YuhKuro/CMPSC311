import tkinter as tk

import stat

import os

import sys

import fcntl

import errno

from datetime import datetime

try:

	os.mkfifo("./pytoc", 0o666)

except FileExistsError:

	pass

try:

	os.mkfifo("./onexit", 0o666)

except FileExistsError:

	pass

def loop():

	io = os.open("ctopy", os.O_RDONLY | os.O_NONBLOCK)

	try:

		buffer = os.read(io,10000)

	except OSError as err:

		if err.errno == errno.EAGAIN or err.errno == errno.EWOULDBLOCK:

			buffer = None

		else:

			raise

	if buffer is None:

		os.close(io)

	else:

		str1 = buffer.decode("utf-8") + '\n'

		if(len(str1) > 2):

			text.insert(tk.END,str1)

	window.after(500,loop)

def handle_click(event):

	fd = os.open("pytoc", os.O_WRONLY)

	str1 = entry.get()

	try:

		os.write(fd, str1.encode())

	except BrokenPipeError:

		pass

	now = datetime.now()

	current_time = now.strftime("[%I:%M %p]")

	text.insert(tk.END,current_time + "Me: " + str1 + "\n")

	entry.delete(0,tk.END)

def on_closing():

	fd = os.open("onexit", os.O_WRONLY)

	str2 = "exit"

	os.write(fd, str2.encode())

	window.destroy()

	



window = tk.Tk()

window.title("Messenger")

frame = tk.Frame(relief=tk.SUNKEN)

text = tk.Text(master=frame)

label = tk.Label(text="Send Message:")

entry = tk.Entry()

sendBtn = tk.Button(window,text="Send")

sendBtn.bind("<Button-1>",handle_click)

frame.pack()

text.pack()

label.pack()

entry.pack()

sendBtn.pack()

loop()

window.protocol("WM_DELETE_WINDOW", on_closing)

window.mainloop()



	