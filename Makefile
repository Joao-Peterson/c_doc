CC = gcc
C_ARGS = -g

build : main.exe

main.exe : build/main.o build/doc.o build/doc_json.o build/base64.o
	${CC} $^ -o $@

build/main.o : main.c
	${CC} ${C_ARGS} -c $< -o $@

build/doc.o : doc/doc.c
	${CC} ${C_ARGS} -c $< -o $@

build/doc_json.o : doc/doc_json.c
	${CC} ${C_ARGS} -c $< -o $@

build/base64.o : base64/base64.c
	${CC} ${C_ARGS} -c $< -o $@

clean : 
	@rm build/main.o
	@rm build/doc.o
	@rm build/doc_json.o
	@rm build/base64.o