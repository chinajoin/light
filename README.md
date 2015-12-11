     light - Simple Epoll Http Server

（在heiyeluren的基础上实现，http://blog.csdn.net/heiyeshuwu）

这是一个支持静态资源访问的http服务器，使用C语言编写。

支持以下功能:

  *  Support GET/HEAD method
  *  The common MIME types.
  *  Support Self custom default index page
  *  Directory listings.
  *  Support access log
  *  Support Self custom port and max clients
  *  Use Epoll (Ankur Shrivastava)
  *  Use Hash-Table (Ankur Shrivastava)

TODO:

  * 使用epoll异步事件监听机制实现
  * 修复了之前版本对内存使用的问题
  * 还未支持POST
  * 集成http-parser库
  * 考虑对多进程（master-worker）的支持


Auther: melanc - whdsmile@gmail.com qq:869314629
