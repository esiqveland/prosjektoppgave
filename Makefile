CCLIBS := -lm
UNAME_S := $(shell uname)
ifeq ($(UNAME_S),Linux)
    CCLIBS += -lrt
endif
ifeq ($(UNAME_S),Darwin)
endif

all:
	mkdir -p bin
	gcc -O3 search/c_port/search.c -o bin/search_bin -ggdb $(CCLIBS)
	gcc -O3 loaddist/load_distributer.c -o bin/load_distributor_bin
clean:
	rm bin/search_bin
	rm bin/load_distributor_bin
