all:
	@gcc http_get.c -o http_get -g -Wall -Wextra -pedantic
	@gcc tcp_server.c -o tcp_server -g -Wall -Wextra -pedantic -lpthread

clean:
	@rm -f http_get tcp_server
