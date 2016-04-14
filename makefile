# minimalist makefile
.SUFFIXES:
#
.SUFFIXES: .cpp .o .c .h

CFLAGS = -fPIC -march=native -std=c99 -O3 -Wall -Wextra -pedantic -Wshadow
LDFLAGS = -shared
LIBNAME=libmaskedvbyte.so.0.0.1
all:  unit $(LIBNAME)
test: 
	./unit
install: $(OBJECTS)
	cp $(LIBNAME) /usr/local/lib
	ln -s /usr/local/lib/$(LIBNAME) /usr/local/lib/libmaskedvbyte.so
	ldconfig
	cp $(HEADERS) /usr/local/include



HEADERS=./include/varintdecode.h ./include/varintencode.h

uninstall:
	for h in $(HEADERS) ; do rm  /usr/local/$$h; done
	rm  /usr/local/lib/$(LIBNAME)
	rm /usr/local/lib/libmaskedvbyte.so
	ldconfig


OBJECTS= varintdecode.o varintencode.o


varintencode.o: ./src/varintencode.c $(HEADERS)
	$(CC) $(CFLAGS) -c ./src/varintencode.c -Iinclude  

varintdecode.o: ./src/varintdecode.c $(HEADERS)
	$(CC) $(CFLAGS) -c ./src/varintdecode.c -Iinclude  



$(LIBNAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(LIBNAME) $(OBJECTS)  $(LDFLAGS) 




example: ./example.c    $(HEADERS) $(OBJECTS)
	$(CC) $(CFLAGS) -o example ./example.c -Iinclude  $(OBJECTS)

unit: ./tests/unit.c    $(HEADERS) $(OBJECTS)
	$(CC) $(CFLAGS) -o unit ./tests/unit.c -Iinclude  $(OBJECTS)
dynunit: ./tests/unit.c    $(HEADERS) $(LIBNAME)
	$(CC) $(CFLAGS) -o dynunit ./tests/unit.c -Iinclude  -lmaskedvbyte 

clean: 
	rm -f unit *.o $(LIBNAME) example
