     tmhttpd - TieMa(Tiny&Mini) Http Server

（在heiyeluren的基础上实现，http://blog.csdn.net/heiyeshuwu）

这是一个支持静态资源访问的http服务器，使用C语言编写。
  
  *  Support GET/HEAD/POST method
  *  The common MIME types.
  *  Support Self custom default index page
  *  Directory listings.
  *  Support access log
  *  Support Self custom port and max clients
  *  ...

1. 使用epoll事件监听机制实现
2. 目前对相应头信息支持存在问题
3. 对epoll的使用目前也存在问题
4. POST支持也存在问题

	melanc - whdsmile@gmail.com qq:869314629
