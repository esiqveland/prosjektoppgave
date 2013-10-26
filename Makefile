CCLIBS := -lm
UNAME_S := $(shell uname)
ifeq ($(UNAME_S),Linux)
    CCLIBS += -lrt
endif
ifeq ($(UNAME_S),Darwin)
endif

all:
	mkdir -p bin
	gcc search/c_port/search.c -o bin/search_bin -ggdb $(CCLIBS)
	gcc search/c_port/stripped_search.c -o bin/search_strip -ggdb
	gcc search/c_port/testing_uthash.c -o bin/search_testing -ggdb
	gcc search/c_port/sender.c -o bin/sender_bin
	gcc loaddist/load_distributer.c -o bin/load_distributor_bin
clean:
	rm bin/search_bin
	rm bin/search_strip
	rm bin/search_testing
	rm bin/sender_bin
	rm bin/load_distributor_bin
