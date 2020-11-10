
test01: File.o test01.o block.o segment.o inode.o
	@gcc io/File.o apps/test01.o io/block.o io/segment.o io/inode.o -o apps/test01
test02: File.o test02.o block.o segment.o inode.o
	@gcc io/File.o apps/test02.o io/block.o io/segment.o io/inode.o -o apps/test02

File.o: io/File.c
	@gcc -c io/File.c -o io/File.o -Wall -Werror

block.o: io/block.c
	@gcc -c io/block.c -o io/block.o -Wall -Werror

segment.o: io/segment.c
	@gcc -c io/segment.c -o io/segment.o -Wall -Werror

inode.o: io/inode.c
	@gcc -c io/inode.c -o io/inode.o -Wall -Werror

test01.o: apps/test01.c
	@gcc -Iio -c apps/test01.c -o apps/test01.o -Wall -Werror

test02.o: apps/test02.c
	@gcc -Iio -c apps/test02.c -o apps/test02.o -Wall -Werror
		
clean:
	rm -rf apps/*.o io/*.o disk/vdisk apps/test01 apps/test02