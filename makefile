all: sender receiver

sender: Sender.o funcOfSender.o server.h
	gcc -Wall Sender.o funcOfSender.o -o sender
Sender.o: Sender.c server.h
	gcc -c -Wall Sender.c -o Sender.o
funcOfSender.o: funcOfSender.c server.h
	gcc -c -Wall funcOfSender.c -o funcOfSender.o
	
	
receiver: Receiver.o
	gcc -Wall Receiver.o -o receiver
Receiver.o: Receiver.c
	gcc -c -Wall Receiver.c -o Receiver.o

clean:
	-rm  *.o sender receiver
