CFLAGS = -Wall -g

all: tour essaim

# -pthread : la bibliothèque des threads, pour la tour
tour: tour.c radar.c commun.c tour.h radar.h commun.h
	gcc $(CFLAGS) -pthread tour.c radar.c commun.c -o tour

essaim: essaim.c commun.c commun.h
	gcc $(CFLAGS) essaim.c commun.c -o essaim

clean:
	rm -f tour essaim
