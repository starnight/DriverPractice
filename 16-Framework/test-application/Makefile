PROJ=test-application

all:
	cc main.c -o $(PROJ)

test:
	sudo ./$(PROJ) /dev/useexample0
	sudo ./$(PROJ) /dev/useexample1
	sudo ./$(PROJ) /dev/useexample2
	sudo ./$(PROJ) /dev/useexample3

clean:
	rm $(PROJ)
