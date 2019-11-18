make clean
make client
retval=$?
proxy_address="localhost"
proxy_port="4950"
# url="http://man7.org/linux/man-pages/man2/send.2.html"
# url="http://man7.org/linux/man-pages/man2/open.2.html"
url="http://man7.org/linux/man-pages/man1/diff.1.html"
# url="http://man7.org/linux/man-pages/man7/regex.7.html"
# url="http://man7.org/linux/man-pages/man2/accept.2.html"
# url="http://man7.org/linux/man-pages/man2/getpeername.2.html"
# url="http://man7.org/linux/man-pages/man2/socket.2.html"
# url="http://man7.org/linux/man-pages/man2/listen.2.html"
# url="http://man7.org/linux/man-pages/man2/bind.2.html"
# url="http://man7.org/linux/man-pages/man7/sctp.7.html"
# url="http://man7.org/linux/man-pages/man7/epoll.7.html"

if [ $retval -eq 0 ]; then
    # clear
    ./client $proxy_address $proxy_port $url
else
    echo MAKE CLIENT ERROR
fi
