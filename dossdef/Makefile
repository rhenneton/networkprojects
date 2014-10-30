all : send.c rec.c
	gcc send.c -o send -lz -lpthread
	gcc rec.c -o rec -lz -lpthread
	
send : send.c 
	gcc send.c -o send -lz -lpthread -fno-stack-protector
	./send  --file input.txt --splr 0 --sber 0 ::1 12345
	
rec : rec.c
	gcc rec.c -o rec -lz -lpthread
	./rec :: 12345
	
clean : 
	rm rec
	rm send	
