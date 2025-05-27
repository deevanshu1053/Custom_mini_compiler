import tkinter as tk
from tkinter import filedialog, scrolledtext, messagebox
import subprocess
import os

class MiniCompilerUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Mini Compiler UI")

        # Source code input
        self.text = scrolledtext.ScrolledText(root, width=80, height=20)
        self.text.pack(padx=10, pady=10)

        # Buttons
        btn_frame = tk.Frame(root)
        btn_frame.pack(pady=5)

        tk.Button(btn_frame, text="Open File", command=self.open_file).pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Print AST", command=self.print_ast).pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Print Intermediate Code", command=self.print_intermediate).pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Print Symbol Table", command=self.print_symbol_table).pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Print Output", command=self.print_output).pack(side=tk.LEFT, padx=5)

        # Output area
        self.output = scrolledtext.ScrolledText(root, width=80, height=20, state=tk.DISABLED)
        self.output.pack(padx=10, pady=10)

        # Temp file for code
        self.temp_file = "ui_temp_input.txt"

    def open_file(self):
        path = filedialog.askopenfilename(filetypes=[("Text Files", "*.txt"), ("All Files", "*.*")])
        if path:
            with open(path, "r") as f:
                self.text.delete(1.0, tk.END)
                self.text.insert(tk.END, f.read())

    def run_compiler(self, mode):
        # Write code to temp file
        with open(self.temp_file, "w") as f:
            f.write(self.text.get(1.0, tk.END))
        # Run the compiler and capture output (both stdout and stderr)
        exe = "./compiler"
        if not os.path.exists(exe):
            messagebox.showerror("Error", "main executable not found. Please build your compiler.")
            return ""
        try:
            proc = subprocess.run([exe, self.temp_file], capture_output=True, text=True, timeout=10)
            output = proc.stdout
            error = proc.stderr
        except Exception as e:
            output = ""
            error = str(e)
        # If there is a parsing error or any stderr, show it
        if error.strip():
            return error
        # Parse output for different sections
        sections = self.split_sections(output)
        return sections.get(mode, "")

    def split_sections(self, output):
        # Split output into sections by headers
        sections = {}
        current = None
        lines = output.splitlines()
        for line in lines:
            if line.startswith("--- Abstract Syntax Tree (AST) ---"):
                current = "ast"
                sections[current] = ""
            elif line.startswith("--- Intermediate Code ---"):
                current = "intermediate"
                sections[current] = ""
            elif line.startswith("--- Symbol Table ---"):
                current = "symbol"
                sections[current] = ""
            elif line.startswith("Output"):
                current = "output"
                sections[current] = ""
            elif line.startswith("---"):
                current = None
            elif current:
                sections[current] += line + "\n"
        return sections

    def print_ast(self):
        result = self.run_compiler("ast")
        self.show_output(result or "No AST output found.")

    def print_intermediate(self):
        result = self.run_compiler("intermediate")
        self.show_output(result or "No intermediate code found.")

    def print_symbol_table(self):
        result = self.run_compiler("symbol")
        self.show_output(result or "No symbol table found.")

    def print_output(self):
        result = self.run_compiler("output")
        self.show_output(result or "No output found.")

    def show_output(self, text):
        self.output.config(state=tk.NORMAL)
        self.output.delete(1.0, tk.END)
        self.output.insert(tk.END, text)
        self.output.config(state=tk.DISABLED)

if __name__ == "__main__":
    root = tk.Tk()
    app = MiniCompilerUI(root)
    root.mainloop()