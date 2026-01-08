#!/usr/bin/env python3
import os
import json

def main():
    root = os.path.abspath(os.getcwd())
    entries = []
    for name in os.listdir(root):
        if name.endswith('.cpp'):
            file_path = os.path.join(root, name)
            cmd = f"g++ -std=c++17 -I{root} -c \"{file_path}\""
            entries.append({
                "directory": root,
                "command": cmd,
                "file": file_path
            })
    out = os.path.join(root, 'compile_commands.json')
    with open(out, 'w', encoding='utf-8') as f:
        json.dump(entries, f, indent=2)
    print(f'Wrote {out} with {len(entries)} entries')

if __name__ == '__main__':
    main()
