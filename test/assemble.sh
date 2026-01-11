# nasm -felf64 Print.asm -o Print.o
nasm -felf64 "$1.asm" -o Main.o
ld "$1.o" Print.o -o "$1"
./"$1"; echo "Exit code:" $?
rm "$1" "$1.o"
