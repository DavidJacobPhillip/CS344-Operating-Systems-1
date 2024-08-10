CC=gcc --std=gnu99 -g -pthread -lm

all: enc_client enc_server dec_client dec_server keygen

enc_client: enc_client.c 
	$(CC) enc_client.c -o enc_client

enc_server: enc_server.c 
	$(CC) enc_server.c -o enc_server

dec_client: dec_client.c 
	$(CC) dec_client.c -o dec_client

dec_server: dec_server.c 
	$(CC) dec_server.c -o dec_server

keygen: keygen.c 
	$(CC) keygen.c -o keygen

clean:
	rm -rf enc_client.o enc_client f enc_client.dSYM enc_server.o enc_server f enc_server.dSYM dec_client.o dec_client f dec_client.dSYM dec_server.o dec_server f dec_server.dSYM keygen.o keygen f keygen.dSYM