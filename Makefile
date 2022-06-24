# -*- MakeFile -*- #

vm: jbb.c
	gcc $< -o $@

testff:
	./vm addresses.txt fifo fifo

testll: 
	./vm addresses.txt lru lru

testfl:
	./vm addresses.txt fifo lru

testlf:
	./vm addresses.txt lru fifo

testerro1:
	./vm addresses.txt parametro errado

testerro2:
	./vm addresses.txt quantidade de parametro

testerro3:
	./vm addresses.txt fifo fifo f

testerro4:
	./vm addresses.txt fifo

testerro5:
	./vm addresses.doc fifo fifo

clean: 
	rm vm
