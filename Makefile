all:
	gcc search/c_port/search.c -o search_bin -ggdb

clean:
	rm search_bin
