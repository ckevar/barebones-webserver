# barebones-webserver
A bare bones web server written in C++ (orignally written for windows)

Based on the [forked repository](https://www.youtube.com/watch?v=YqEqjODUkWY&t=7s); however, this WebServer has been modified for running on Linux (ubuntu/pop-os) and uses [**poll()**](http://man7.org/linux/man-pages/man2/poll.2.html) instead of [**select()** ](http://man7.org/linux/man-pages/man2/select.2.html).

the Maximun number of clients is MAX_CLIENTS = 10 and can be modified in Tcplistener.h code. Due to **poll()** is being used, the maximun recomended is *1000*, to reach further number of clients, it can be modified to use **epoll()** for having between 1000 to 10000 active file descriptor.  