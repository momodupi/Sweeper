motor:motor.o
	gcc -o motor motor.o -lwiringPi -lpthread
motor.o:motor.c motor.h pin.h
	gcc -c motor.c
clean:rm -f *.o main
