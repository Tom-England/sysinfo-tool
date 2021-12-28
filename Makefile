install:
	gcc -o tfetch src/main.c
	mv tfetch usr/bin/


clean:
	rm tfetch
