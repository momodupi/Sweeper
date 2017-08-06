sweeper: motor.o camera.o sweeper.o data.o
	gcc -o sweeper sweeper.o motor.o camera.o data.o -lwiringPi -lpthread
data.o: data.c data.h
	gcc -c data.c
motor.o: motor.c motor.h pin.h
	gcc -c motor.c
camera.o: camera.c camera.h
	gcc -c camera.c
sweeper.o: sweeper.c motor.h camera.h data.h
	gcc -c sweeper.c
clean: rm -f *.o main
