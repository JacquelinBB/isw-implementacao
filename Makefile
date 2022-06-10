# -*- MakeFile -*- #

vm: jbb_final_ate_4.c
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
	./vm addresses.txt quero nota

testerro2:
	./vm addresses.txt erro

clean: 
	rm vm
