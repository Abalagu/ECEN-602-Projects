clear
make clean
make
if [ $? -eq 0 ]
then
    clear
    ./server 4950
fi
