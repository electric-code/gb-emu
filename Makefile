SOURCES = cpu.c gb.c
OBJECTS = $(SOURCES:.c=.o)
INCLUDES = cpu.h
BINARY = gb
# CCFLAGS = -DDEBUG=100

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $^

%.o: %.c $(INCLUDES)
	$(CC) -c -o $@ $(CCFLAGS) $<

.PHONY: clean
clean:
	$(RM) $(BINARY)
	$(RM) $(OBJECTS)
