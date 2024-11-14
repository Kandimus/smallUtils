import sys
import datetime
import PySide6.QtCore as qtcore
import PySide6.QtWidgets as qtwgt
import PySide6.QtGui as qtgui

import tkinter as tk
from tkinter import filedialog as fd

# gPath = './'
gPath = '../../../../runtime/logs'

class LogItem:
    date = datetime
    name = str
    level = int
    file = str
    lineno = int
    text = str
    
    def __init__(self):
        self.date = datetime.datetime.now()
        self.name = ''
        self.level = 0
        self.file = ''
        self.lineno = -1
        self.text = ''
    

class MainFrame:
    rawLines = []
    logLines = []
    levelColor = dict
    
    def __init__(self):
        super().__init__()
        
        self.levelColor = {'E': '#FF0000', 'W': '#FFFF00', 'I': '#FFFFFF', 'N': '#DDDDDD', 'D': '#f0ddb6'}
        
        self.__root = tk.Tk()
        self.__root.title("SmallUtils::LogViewer")
        self.__root.geometry("800x600")
        
        self.__open = tk.Button(self.__root, text="Open", command=self.openFile)
        self.__open.pack()
        
        self.__edit = tk.Entry(self.__root)
        self.__edit.pack(fill=tk.X)
        
        self.__list = tk.Listbox(self.__root, font=("Courier New", 12), selectmode=tk.SINGLE)
        self.__list.pack(side=tk.LEFT, expand=True, fill=tk.BOTH)
        
        self.__scrollbar = tk.Scrollbar(orient="vertical", command=self.__list.yview)
        self.__scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.__list["yscrollcommand"] = self.__scrollbar.set
        
    def run(self):
        self.__root.mainloop()
    
    def openFile(self):
        filetypes = (
            ('Log files', '*.log'),
            ('All files', '*.*')
        )
        filename = fd.askopenfilename(filetypes=filetypes)
        if len(filename) == 0:
            return
        
        self.loadFile(filename)
        self.showLines()
    
    def loadFile(self, filename: str):
        self.logLines.clear()
        
        with open(str(filename)) as file:
            while line := file.readline():
                index = line.find('] ')
                if index == -1:
                    continue
                
                info = line[21:index]
                infoList = info.split(':')
                
                item = LogItem()
                item.date = datetime.datetime.strptime(line[:19], '%d.%m.%Y %H:%M:%S')
                item.name = infoList[0]
                item.level = infoList[1]
                item.file = infoList[2]
                item.lineno = infoList[3]
                item.text = line[index + 2:len(line) - 1]
                self.logLines.append(item)
    
    def showLines(self):
        self.__list.delete(0, tk.END)
        
        for item in self.logLines:
            text = ''
            text += datetime.datetime.strftime(item.date, '%H:%M:%S')
            text += ' '
            text += item.text
            
            index =self.__list.size()
            self.__list.insert(index, text)
            self.__list.itemconfig(index, bg=self.levelColor[item.level], foreground="#000000") #foreground=

# Main
if __name__ == "__main__":
    if len(sys.argv) > 1:
        gPath = sys.argv[1]
    
    print(tk.TkVersion)

    mainFrame = MainFrame()
    mainFrame.run()
