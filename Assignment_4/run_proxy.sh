# clear
make clean
make proxy
retval=$?
local_address="localhost"
local_port="4950"
if [ $retval -eq 0 ]; then
    # clear
    ./proxy $local_address $local_port
else
    echo MAKE PROXY ERROR
fi
