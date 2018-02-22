NAME = main
EMULATOR = fceux

build:
	cc65 -Oi $(NAME).c --add-source
	ca65 $(NAME).s
	ca65 reset.s
	ld65 -C nes.cfg -o $(NAME).nes reset.o $(NAME).o nes.lib

clean:
	rm ./*.o
	rm ./$(NAME).s
	rm ./$(NAME).nes

play: $(NAME).nes
	$(EMULATOR) $(NAME).nes