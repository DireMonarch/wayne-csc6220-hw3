all: matrix

matrix: jim-haslett-csc6220-hw3.c
	gcc -lpthread -o jim-haslett-csc6220-hw3 jim-haslett-csc6220-hw3.c

clean:
	rm jim-haslett-csc6220-hw3 result.txt

lint:
	cpplint jim-haslett-csc6220-hw3.c

run:
	./jim-haslett-csc6220-hw3
