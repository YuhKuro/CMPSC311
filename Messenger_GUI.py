import tkinter as tk

import stat

import os

import fcntl

from socket import*

try:

	os.mkfifo("./pytoc", 0o666)

except FileExistsError:

	pass

def loop():

	with open('send.txt') as f:

		str1 = ''.join(f.readlines())

		str1.replace("{", "")

		str1.replace("}", "")

		text.insert(tk.END,str1)

		f.close()

	with open("send.txt", 'r+') as f:

    		f.truncate(0)

    		f.close()

	window.after(500,loop)

def handle_click(event):

	#with open("recieve.txt", 'w') as f:

		#f.write(entry.get())

		#f.close()

	fd = os.open("pytoc", os.O_WRONLY)

	str1 = entry.get() +'\n'

	os.write(fd, str1.encode())

	entry.delete(0,tk.END)

	



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

window.mainloop()

	
