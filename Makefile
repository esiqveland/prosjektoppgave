all:
	gcc search/c_port/search.c -o search_bin -ggdb
	gcc search/c_port/stripped_search.c -o search_strip -ggdb
	gcc search/c_port/testing_uthash.c -o search_testing -ggdb
	gcc search/c_port/sender.c -o sender_bin
	gcc loaddist/load_distributer.c -o load_distributor_bin
clean:
	rm search_bin
