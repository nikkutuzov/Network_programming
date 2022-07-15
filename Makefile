all: build_TCP_server build_UDP_sender build_UDP_receiver

build_TCP_server:
	gcc -o TCP_server TCP_server.c

build_UDP_sender:
	gcc -o UDP_sender UDP_sender.c

build_UDP_receiver:
	gcc -o UDP_receiver UDP_receiver.c
