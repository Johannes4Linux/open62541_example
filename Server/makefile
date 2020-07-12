myServer: ../open62541/open62541.o myServer.o
	gcc ../open62541/open62541.o myServer.o -o myServer

../open62541/open62541.o: ../open62541/open62541.c
	gcc -c -std=c99 ../open62541/open62541.c -o ../open62541/open62541.o

myServer.o: myServer.c
	gcc -c myServer.c

clean:
	rm *.o myServer

run:
	./myServer
