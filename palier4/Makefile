CFLAGS = -Wall -g

all: tour essaim course

# -pthread : la bibliothèque des threads, pour la tour (et pour course)
tour: tour.c radar.c commun.c tour.h radar.h commun.h
	gcc $(CFLAGS) -pthread tour.c radar.c commun.c -o tour

essaim: essaim.c commun.c commun.h
	gcc $(CFLAGS) essaim.c commun.c -o essaim

course: course.c
	gcc $(CFLAGS) -pthread course.c -o course

clean:
	rm -f tour essaim course
