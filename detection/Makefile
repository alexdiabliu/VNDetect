# Host (stub) build for the modular MSP detection skeleton.
# On-target MSP builds should replace CC/CFLAGS and link real driver .c files.

ROOT := $(abspath .)
CC   ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -Idetection -I$(ROOT) -DMSP_HOST_BUILD
# When invoked as `make -C detection`, include path is this directory:
CFLAGS := -std=c11 -Wall -Wextra -Werror -I. -DMSP_HOST_BUILD

SRCS := \
	main.c \
	protocol/status_protocol.c \
	ports/port_registry.c \
	platform/board_bind.c \
	app/features.c \
	app/think/think.c \
	drivers/uart/uart_stub.c \
	drivers/rgb/rgb_sensor_stub.c \
	drivers/sensor_x/sensor_x_stub.c \
	drivers/status/status_transport_stub.c

OBJS := $(SRCS:.c=.o)
BIN  := detection_host

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(OBJS) $(BIN)
