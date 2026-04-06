CC     = gcc
CFLAGS = -Wall -Wextra -g
SRC    = src/main.c \
         src/tree.c \
         src/scan_go.c \
         src/analyse_go.c \
         src/scan_gpl.c \
         src/analyse_gpl.c \
         src/pcode.c \
         src/exec.c
OUT    = compilo

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)