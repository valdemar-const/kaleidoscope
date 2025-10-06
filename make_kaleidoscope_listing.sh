#!/usr/bin/env bash

# Проверяем, что переданы аргументы
if [[ $# -eq 0 ]]; then
    echo "Usage: $0 <directory> [<directory> ...]"
    exit 1
fi

# Перебираем все аргументы‑каталоги
for dir in "$@"; do
    # Тело цикла оставляем без изменений
    $(bash ./full_md_listing.sh $dir >> kaleidoscope_src.md)
done
