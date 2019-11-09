make clean
make proxy
retval=$?

if [ $retval -eq 0 ]; then
    clear
    ./proxy
else
    echo MAKE PROXY ERROR
fi
