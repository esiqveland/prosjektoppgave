all:
	gcc search/c_port/search.c -o search_bin -g

clean:
	rm search_bin
