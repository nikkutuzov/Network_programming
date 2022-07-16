all: build_TCP_server build_TCP_client build_UDP_sender build_UDP_receiver build_Epoll_server build_libev_server

build_TCP_server:
	gcc -o TCP_server TCP_server.c

build_TCP_client:
	gcc -o TCP_client TCP_client.c

build_UDP_sender:
	gcc -o UDP_sender UDP_sender.c

build_UDP_receiver:
	gcc -o UDP_receiver UDP_receiver.c

build_Epoll_server:
	gcc -o Epoll_server Epoll_server.c

build_libev_server:
	gcc -o libev_server libev_server.c -lev
