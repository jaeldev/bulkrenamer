# Bulk Renamer

A simple CLI bulk file renamer written in C++.

Bulk Renamer help you rename multiple files at once, and with a preview so you can check new name before applying any change.

## Features

- Process files in the selected folder
- Skips hidden items where the name start with dot
- Replaces spaces in names with underscores
- Optionally add custom prefix
- Optionally add sequential numbering, starting at `001`
- Previews the new name and asks for confirmation before renaming

## Usage

1. Add folder path of the selected folder.
2. Choose whether to add a prefix and/or sequential numbering.
3. Review the preview of the new filenames.
4. Apply the changes when you’re ready.

## Build

Compile with C++20 compatible compiler:
```bash
g++ -std=c++20 main.cpp -o bulkrenamer
```

It makes an executable called 'bulkrenamer'. You can name it however you like.

## License

This project is licensed under GNU General Public License version 2.0. Please see the LICENSE file for license details.

## Thank You
Thank you for checking out Bulk Renamer. I hope you find this cool! I don't know where is to receive feedback, or suggestions, but they are welcome.
