# Bulk Renamer

A simple CLI bulk file renamer written in C++.

Bulk Renamer help you rename multiple files at once, it only shows a preview of the old names and the new names, nothing in disk is changed
until you add `--execute`.

## Features

- Renames the files in a folder you choose
- Skips hidden items where the name start with dot and subfolders
- Replaces spaces in names with underscores
- Optionally add custom prefix
- Optionally add sequential numbering, starting at `001`
- Handles file name collisions
- Previews first, apply only with `--execute`

## Build

You need a C++17 or newer compiler. It has tested with g++ on Linux using C++17 and C++20.
```bash
g++ -std=c++20 main.cpp -o bulkrenamer
```
It makes an executable called 'bulkrenamer'. You can name it however you like.

## Usage

```bash
bulkrenamer --dir <path> [--prefix <text>] [--number] [--execute]
```

## License

This project is licensed under GNU General Public License version 3.0. Please see the [LICENSE](LICENSE) file for license details.

## Thank You
Thank you for checking out Bulk Renamer. I hope you find this cool! I don't know where is to receive feedback, or suggestions, but they are welcome.
