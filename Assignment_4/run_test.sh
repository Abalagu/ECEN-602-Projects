make clean
make client
retval=$?
proxy_address="localhost"
proxy_port="4950"
declare -a arr=(
    "http://man7.org/linux/man-pages/man2/send.2.html"
    "http://man7.org/linux/man-pages/man2/send.2.html"
    "http://man7.org/linux/man-pages/man2/open.2.html"
    "http://man7.org/linux/man-pages/man1/diff.1.html"
    "http://man7.org/linux/man-pages/man7/regex.7.html"
    "http://man7.org/linux/man-pages/man2/accept.2.html"
    "http://man7.org/linux/man-pages/man2/getpeername.2.html"
    "http://man7.org/linux/man-pages/man2/socket.2.html"
    "http://man7.org/linux/man-pages/man2/listen.2.html"
    "http://man7.org/linux/man-pages/man2/bind.2.html"
    "http://man7.org/linux/man-pages/man7/sctp.7.html"
    "http://man7.org/linux/man-pages/man7/epoll.7.html"
    "http://man7.org/linux/man-pages/man7/epoll.7.html"
)

if [ $retval -eq 0 ]; then
    clear
    for i in "${arr[@]}"; do
        url=$i
        ./client $proxy_address $proxy_port $url
    done

else
    echo MAKE CLIENT ERROR
fi
