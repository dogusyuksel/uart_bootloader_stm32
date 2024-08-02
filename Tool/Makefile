CC = gcc
RM = rm -rf
CFLAGS = -Wall -Wextra -g3 -O0 -lpthread -Wno-deprecated-declarations
all_c_files := $(wildcard *.c)

.PHONY: all clean

all:
	for file in $(all_c_files); do \
		$(CC) $$file -o $$file".exe" $(CFLAGS); \
	done

partial:
	$(CC) $(FILE) -o $(FILE)".exe" $(CFLAGS); \

clean:
	for file in $(all_c_files); do \
		$(RM) -rf $$file".exe"; \
	done
