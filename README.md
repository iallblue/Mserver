# Mserver
***

> 基本信息

*	C语言实现简易版http server,目前支持GET,POST,CGI程序
*	cgi-bin 目录主要包含cig程序
*	static	目录主要是静态资源



> 运行方式

*	`./mserver 8088`

> 函数索引

*	`doit()` - 处理web请求,所有请求过程处理的入口
*	`read_requesthdrs()` - 处理请求行，可以解析出post报文的Content-length
*	`parse_uri()` - 解析uri，将uri转换为请求资源所在的路径，同时获得get请求的参数，设置环境变量query_string
*	`serve_static()` - 提供对静态资源请求的响应
*	`serve_dynamic()` - 提供动态资源请求的响应(cgi程序)
*	`post_dynamic()` - 响应post请求，获得post请求参数，请求CGI返回
*	`get_filetype()` - 获取请求静态资源的后缀名-文件类型
*	`client_error()` - 错误处理函数 

> 感受

*	为了增加post请求处理，浪费了好多时间，试了好多方法，最后才发现别人用的 pipe。
*	还是自己懂得太少了，关于tcp了解不够深入

