CC = gcc
C_ARGS = -g

build : main.exe clear_obj

main.exe : main.o doc/doc.o doc/doc_json.o
	${CC} $^ -o $@

main.o : main.c
	${CC} ${C_ARGS} -c $< -o $@

doc/doc.o : doc/doc.c
	${CC} ${C_ARGS} -c $< -o $@

doc/doc_json.o : doc/doc_json.c
	${CC} ${C_ARGS} -c $< -o $@

clear_obj : 
	@rm main.o
	@rm doc/doc.o
	@rm doc/doc_json.o