




main: ftpclient.c str.c srch.c
		gcc -g -Wall -DPRINTDEBUG -o ftpclient ftpclient.c str.c srch.c


.PHONY: clean
	clean:
		rm -rf *.o
