make clean
make client
retval=$?
proxy_address="localhost"
proxy_port="4950"
url="http://man7.org/linux/man-pages/man2/send.2.html"
if [ $retval -eq 0 ]; then
    clear
    ./client $proxy_address $proxy_port $url
else
    echo MAKE CLIENT ERROR
fi
