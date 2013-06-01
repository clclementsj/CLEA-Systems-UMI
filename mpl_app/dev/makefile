all:	dwgpart

dwgpart:	dwgpart.c dwgpart.h
	gcc dwgpart.c -I . -o dwgpart

bup: 
	cp dwgpart* makefile config.mpl *.sql backup

rbup:
	scp -P 2222 ~/dev/backup/* cliff@23.30.3.180:~/dev
