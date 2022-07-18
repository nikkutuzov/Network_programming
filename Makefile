all: build_TCP_server build_TCP_client build_UDP_sender build_UDP_receiver build_Epoll_server build_libev_server

build_TCP_server:
	gcc -o TCP_server 1_1_TCP_server.c

build_TCP_client:
	gcc -o TCP_client 1_2_TCP_client.c

build_UDP_sender:
	gcc -o UDP_sender 2_2_UDP_sender.c

build_UDP_receiver:
	gcc -o UDP_receiver 2_1_UDP_receiver.c

build_Epoll_server:
	gcc -o Epoll_server 3_1_Epoll_server.c

build_libev_server:
	gcc -o libev_server 4_LIBEV_server.c -lev
