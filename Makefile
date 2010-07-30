all:
	slice2cpp -I/usr/local/share/Ice ../Murmur.ice
	c++ -I . -I/usr/local/include -c Murmur.cpp 
	c++ -Wall -I . -I/usr/local/include -c mutter.cpp
	c++ -Wall -o mutter Murmur.o mutter.o -L/usr/local/lib -lIce -lIceUtil -liconv -lpthread
