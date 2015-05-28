# p2pPSE : fichier Makefile
#

P2PPSE_DIR = .
P2PPSE_BIN = ${P2PPSE_DIR}/bin
P2PPSE_LIB = ${P2PPSE_DIR}/lib
P2PPSE_SRC = ${P2PPSE_DIR}/src
P2PPSE_OBJ = ${P2PPSE_DIR}/obj
P2PPSE_INCL_DIR = ${P2PPSE_DIR}/include

LIBRARY		=	$(P2PPSE_LIB)/libpse.a
LIB_SRC	=	${P2PPSE_SRC}/datathread.c ${P2PPSE_SRC}/erreur.c ${P2PPSE_SRC}/ligne.c ${P2PPSE_SRC}/msg.c ${P2PPSE_SRC}/msgbox.c ${P2PPSE_SRC}/resolv.c
LIB_OBJ	=	${P2PPSE_OBJ}/datathread.o ${P2PPSE_OBJ}/erreur.o ${P2PPSE_OBJ}/ligne.o ${P2PPSE_OBJ}/msg.o ${P2PPSE_OBJ}/msgbox.o ${P2PPSE_OBJ}/resolv.o

SRV_SRC	=	${P2PPSE_SRC}/serveur.c
CLI_SRC	=	${P2PPSE_SRC}/client.c
SRV_BIN	=	${P2PPSE_BIN}/serveur
CLI_BIN	=	${P2PPSE_BIN}/client

CFLAGS = -g -I. -I${P2PPSE_INCL_DIR} -Wall 
		
LDLIBS = -L${P2PPSE_LIB} -lpse -lm -pthread

CC = gcc

all: libpse client serveur

libpse: $(LIB_OBJ)
	rm -f $(LIBRARY)
	ar rs $(LIBRARY) $(LIB_OBJ)
	rm -f $(LIB_OBJ)

obj/%.o: src/%.c
	gcc -c ${CFLAGS} -o $@ $<

client:
	$(CC) $(CFLAGS) -o $(P2PPSE_BIN)/client $(CLI_SRC)

serveur:
	$(CC) $(CFLAGS) -o $(P2PPSE_BIN)/serveur $(SRV_SRC)

clean:
	rm -f $(LIBOBJ) $(LIBRARY) $(SRV_BIN) $(CLI_BIN) *~