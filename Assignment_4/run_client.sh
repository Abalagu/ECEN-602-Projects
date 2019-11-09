make clean
make client
retval=$?

if [ $retval -eq 0 ]; then
    clear
    ./client
else
    echo MAKE CLIENT ERROR
fi
