import tkinter as tk
import sys
import threading


class StdPrintRedirector:
    def __init__(self, widget, tag="stdout"):
        self.widget = widget
        self.tag = tag

    def write(self, string):
        self.widget.configure(state="normal")
        self.widget.insert("end", string, (self.tag,))
        if self.widget.count('0.0', 'end', 'lines')[0] > 20:
            self.widget.delete('0.0', f'2.0')
        self.widget.configure(state="disabled")


class Console(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        toolbar = tk.Frame(self)
        toolbar.pack(side="top", fill="x")
        b3 = tk.Button(self, text="disable print", command=self.disable_print)
        b3.pack(in_=toolbar, side="left")
        self.text = tk.Text(self, wrap="word")
        self.text.pack(side="top", fill="both", expand=True)
        self.text.tag_configure("stderr", foreground="#b22222")

        self._stdout = sys.stdout
        self._stderr = sys.stderr

        sys.stdout = StdPrintRedirector(self.text, "stdout")
        sys.stderr = StdPrintRedirector(self.text, "stderr")

    def disable_print(self):
        print('Disabling printing of', __file__)
        sys.stdout = self._stdout
        sys.stderr = self._stderr


class Runner:
    app = None
    t = None

    @classmethod
    def close(cls):
        if cls.app:
            cls.app.disable_print()
            try:
                cls.app.quit()
                cls.app.destroy()
            except RuntimeError as excp:
                print(excp)
            cls.app = None

        if cls.t:
            cls.t.join()
            cls.t = None

    @classmethod
    def open(cls):
        cls.t = threading.Thread(
            target=Runner(),
        )
        cls.t.start()

        while not cls.app:
            pass

    def __call__(self):
        Runner.app = Console()
        Runner.app.mainloop()


def main():
    Runner.open()
