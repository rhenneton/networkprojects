all : send.c rec.c
	gcc send.c -o send -lz -lpthread
	gcc rec.c -o rec -lz -lpthread
	
send : send.c 
	gcc send.c -o send -lz -lpthread
	./send --file projectstatement.pdf --sber 20 --splr 500 romain-UX303LA 12345
	
rec : rec.c
	gcc rec.c -o rec -lz -lpthread
	./rec --file projet.pdf 12345
	
clean : 
	rm rec
	rm send	
