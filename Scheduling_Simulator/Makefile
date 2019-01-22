TARGETS = scheduling_simulator
CC = gcc
CFLAGS += -g -std=gnu99 -Wall
OBJS = scheduling_simulator.o task.o

GIT_HOOKS := .git/hooks/applied

all:$(GIT_HOOKS) $(TARGETS)
	
$(TARGETS):$(OBJS)
	$(CC) $(CFLAGS) -o scheduling_simulator $(OBJS)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(GIT_HOOKS):
	@.githooks/install-git-hooks
	@echo

clean:
	rm -rf *.o scheduling_simulator
astyle:
	astyle --style=linux --indent=tab --max-code-length=80 --suffix=none task.c task.h scheduling_simulator.c scheduling_simulator.h
