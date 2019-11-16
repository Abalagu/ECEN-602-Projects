make clean
clear
mode=$1 # get first cmd arg
local_address="localhost"
local_port="4950"
# if with mode flag, then run debug  mode
if [[ -n "$mode" ]]; then
    make proxy_debug
    echo "Debug proxy server"
    gdb --args proxy $local_address $local_port
else # else with no flag, launch in normal mode
    make proxy
    retval=$?
    if [ $retval -eq 0 ]; then
        echo "Launch proxy server"
        ./proxy $local_address $local_port
    else
        echo MAKE PROXY ERROR
    fi
fi
