run:
	gcc -Wall test.c test2.c -o main.bin -g -ffreestanding -Wextra -fno-exceptions -m32 -fno-pie -I ./ -I ./../../../zapps
	make clean

fullclean:
	make clean
	rm -Rf ./*.bin

clean:
	rm -Rf ./*.full
	rm -Rf ./*.o
	rm -Rf ./*.pe