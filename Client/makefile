myClient: ../open62541/open62541.o myClient.o
	gcc ../open62541/open62541.o myClient.o -o myClient

../open62541/open62541.o: ../open62541/open62541.c
	gcc -c -std=c99 ../open62541/open62541.c -o ../open62541/open62541.o

myClient.o: myClient.c
	gcc -c myClient.c

clean:
	rm *.o myClient

run:
	./myClient
