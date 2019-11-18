make clean
make client
retval=$?
proxy_address="localhost"
proxy_port="4950"

# long page 1 for simultaneous connection test
url="http://man7.org/linux/man-pages/man1/bash.1.html"

if [ $retval -eq 0 ]; then
    # clear
    ./client $proxy_address $proxy_port $url
else
    echo MAKE CLIENT ERROR
fi
