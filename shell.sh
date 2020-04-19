#! /bin/bash
gcc shell2.c -L/usr/include -lreadline -o a.out
gnome-terminal -e ./a.out
