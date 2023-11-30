all: rs.out sp.out cs.out

rs.out: restaurant.o connection.o defs.o besides.o json.o loger.o ansi-color.o
	g++  restaurant.o connection.o defs.o besides.o json.o loger.o ansi-color.o -o rs.out

sp.out: supplier.o connection.o defs.o besides.o loger.o ansi-color.o
	g++  supplier.o connection.o defs.o besides.o loger.o ansi-color.o -o sp.out

cs.out: customer.o connection.o defs.o besides.o json.o loger.o ansi-color.o
	g++  customer.o connection.o defs.o besides.o json.o loger.o ansi-color.o -o cs.out	

restaurant.o: restaurant.c defs.h connection.h besides.h json.h loger.h ansi-color.h
	g++  -c restaurant.c
	
connection.o: connection.c connection.h defs.h loger.h
	g++  -c connection.c

besides.o: besides.c besides.h defs.h ansi-color.h
	g++  -c besides.c

defs.o: defs.c defs.h
	g++  -c defs.c

json.o: json.c json.h defs.h
	g++  -c json.c

loger.o: loger.c loger.h defs.h ansi-color.h
	g++  -c loger.c

ansi-color.o: ansi-color.c ansi-color.h
	g++  -c ansi-color.c

supplier.o: supplier.c defs.h connection.h besides.h loger.h ansi-color.h
	g++  -c supplier.c

customer.o: customer.c defs.h connection.h besides.h json.h loger.h ansi-color.h
	g++  -c customer.c

.PHONY: clean
clean:
	rm *.o rs.out sp.out cs.out