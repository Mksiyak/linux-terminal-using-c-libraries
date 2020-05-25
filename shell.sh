#! /bin/bash
gcc shell.c -L/usr/include -lreadline -o a.out
gnome-terminal -e ./a.out
rm ./a.out