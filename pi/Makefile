FILES=sample sound xcorr main
CPP=

HDRS=$(addsuffix .h, $(FILES)) conf.h
SRCS=$(addsuffix .c, $(FILES))
OBJS=$(addprefix obj/, $(addsuffix .o, $(FILES)))

obj/%.o: %.c $(HDRS)
	mkdir -p $(@D)
	gcc -g -c -lm -pthread $< -o $@

fft/obj/%.o: fft/%.cpp fft/DFT.h
	mkdir -p $(@D)
	g++ -g -c $< -o $@

obj/fft.a: fft/obj/DFT.o fft/obj/wrap.o
	mkdir -p $(@D)
	ar rvs $@ $^

backend: $(OBJS) obj/fft.a
	g++ -g -lm -pthread $^ -o $@
	#sudo cp $@ /var/www

.DEFAULT_GOAL=backend
