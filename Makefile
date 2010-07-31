INCPATH		= -I. -I/usr/local/share/Ice -I/usr/local/include
CFLAGS		= -Wall
LIBS		= -L/usr/local/lib -lIce -lIceUtil -lpthread -liconv

all: Murmur.o main.cpp
	c++ $(CFLAGS) -o mutter Murmur.o main.cpp $(INCPATH) $(LIBS)

Murmur.cpp:
	slice2cpp $(INCPATH) Murmur.ice

Murmur.o: Murmur.cpp
	c++ $(INCPATH) -c Murmur.cpp 

clean:
	rm -f *~ Murmur.o Murmur.cpp Murmur.h mutter mutter.core
