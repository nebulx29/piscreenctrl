CC=gcc
LIBS=-lm -lwiringPi
CFLAGS=
BINARY=piscreenctrl
INSTALLPATH=/home/pi/bin

compile:
	$(CC) $(LIBS) $(CFLAGS) -o $(BINARY) piscreenctrl.c

install:
	cp $(BINARY) $(INSTALLPATH)/$(BINARY)
	chmod +x $(INSTALLPATH)/$(BINARY)

clean:
	rm -f *.o
	rm -f $(BINARY)
	rm -f $(INSTALLPATH)/$(BINARY)
