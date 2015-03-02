all:: rs

boot:
	make -C ../llvm boot
	cp ../llvm/boot .

rs: rs.c boot
	gcc -I /usr/local/include/libr $< -o $@ -lr_bin -lr_core -lr_util

.phony: clean

clean:
	rm -f myboot *.o rs
