# nasm -felf64 Print.asm -o Print.o
nasm -felf64 "$1.asm" -o "$1.o"
ld "$1.o" print.o -o "$1"
./"$1"; echo "Exit code:" $?
rm "$1" "$1.o"
