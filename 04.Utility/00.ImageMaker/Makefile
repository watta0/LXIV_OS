all: ImageMaker

ImageMaker: ImageMaker.c 
	gcc -o ../../$@ $<
	chmod 700 ../../$@

clean:
	rm -f ImageMaker
	rm -f ../../ImageMaker