FILES=sample sound xcorr

HDRS=$(addsuffix .h, $(FILES))
SRCS=$(addsuffix .c, $(FILES))
OBJS=$(addprefix obj/, $(addsuffix .o, $(FILES)))

obj/%.o: %.c $(HDRS)
	mkdir -p $(@D)
	gcc -g -c -lm -pthread $< -o $@

backend: $(OBJS) main.c
	gcc -g -lm -pthread $^ -o $@

.DEFAULT_GOAL=backend
