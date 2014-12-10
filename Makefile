CC = gcc

CXX = g++

CFLAGS = -g -Wall -W

arpmock:arpmock.o -lnet
	$(CC) -o $@ $^

arpmock.o:arpmock.c
	$(CC) -c $<

arpsendrcv:arpsendrcv.o
	$(CC) -o $@ $^

arpsendrcv.o:arpsendrcv.c
	$(CC) -c $<

client:client.o
	$(CC) -o $@ $^

client.o:client.c
	$(CC) -c $<

endian:endian.o
	$(CC) -o $@ $^

endian.o:endian.c
	$(CC) -c $<

fork_execl:fork_execl.o
	$(CC) -o $@ $^

fork_execl.o:fork_execl.c
	$(CC) -c $<

getmac:getmac.o
	$(CC) -o $@ $^

getmac.o:getmac.c
	$(CC) -c $<

malloc:malloc.o
	$(CC) -o $@ $^

malloc.o:malloc.c
	$(CC) -c $<

mutilthread:mutilthread.o
	$(CC) -o $@ $^ -lpthread

mutilthread.o:mutilthread.c
	$(CC) -c $<

sendrcvarp:sendrcvarp.o
	$(CC) -o $@ $^

sendrcvarp.o:sendrcvarp.c
	$(CC) -c $<

server:server.o
	$(CC) -o $@ $^
	
server.o:server.c
	$(CC) -c $<

const:const.o
	$(CXX) -o $@ $^
	
const.o:const.cpp
	$(CXX) -c $<

smc_decrypt:smc_decrypt.o spc_smc_decrypt.o
	$(CC) -g -o $@ $^

smc_decrypt.o:smc_decrypt.c spc_smc_decrypt.c spc_smc_decrypt.h
	$(CC) -g -c $^

spc_smc_decrypt.o:spc_smc_decrypt.c spc_smc_decrypt.h
	$(CC) -g -c $^

test_spc_smc_decrypt:test_spc_smc_decrypt.o spc_smc_decrypt.o
	$(CC) -g -o $@ $^

test_spc_smc_decrypt.o:test_spc_smc_decrypt.c spc_smc_decrypt.h
	$(CC) -g -c $^

test_gpg:test_gpg.o
	$(CC) -g -o $@ $^ -lgpgme

test_gpg.o:test_gpg.c
	$(CC) -g -c $^ -D_FILE_OFFSET_BITS=64

test_genrsa:test_genrsa.o
	$(CC) -o $@ $^ -lgcrypt

test_genrsa.o:test_genrsa.c
	$(CC) -c $^

test_encdec:test_encdec.o
	$(CC) $^ -o $@ -lgcrypt

test_encdec.o:test_encdec.c
	$(CC) -c $^

signal:signal.o
	$(CC) $^ -o $@

signal.o:signal.c
	$(CC) -c $^

test_bindwidth_server: test_bindwidth_server.o
	$(CC) $^ -o $@ 

test_bindwidth_server.o:test_bindwidth_server.c
	$(CC) $(CFLAGS) -c $^

test_bindwidth_client: test_bindwidth_client.o
	$(CC) $^ -o $@

test_bindwidth_client.o:test_bindwidth_client.c
	$(CC) $(CFLAGS) -c $^

test_interrupt: test_interrupt.o
	$(CC) $^ -o $@

test_interrupt.o:test_interrupt.c
	$(CC) $(CFLAGS) -c $^

test_soap_send: test_soap_send.o
	$(CC) $^ -o $@

test_soap_send.o:test_soap_send.c
	$(CC) $(CFLAGS) -c $^

test_local_server: test_local_server.o
	$(CC) $^ -o $@

test_local_server.o:test_local_server.c
	$(CC) $(CFLAGS) -c $^

test_local_client: test_local_client.o
	$(CC) $^ -o $@

test_local_client.o:test_local_client.c
	$(CC) $(CFLAGS) -c $^

https_get: https_get.o
	$(CC) $^ -o $@ -lssl -lcrypto

https_get.o:https_get.c
	$(CC) $(CFLAGS) -c $^

all:arpmock arpsendrcv client endian fork_execl getmac malloc mutilthread sendrcvarp server const smc_decrypt test_spc_smc_decrypt test_gpg test_genrsa test_encdec signal \
	test_bindwidth_server test_bindwidth_client test_interrupt test_soap_send test_local_server test_local_client https_get

clean:
	rm arpmock arpmock.o arpsendrcv arpsendrcv.o client client.o endian endian.o fork_execl fork_execl.o getmac getmac.o malloc malloc.o mutilthread mutilthread.o \
	sendrcvarp sendrcvarp.o server server.o const const.o smc_decrypt smc_decrypt.o test_spc_smc_decrypt test_spc_smc_decrypt.o test_gpg test_gpg.o  test_genrsa \
	test_genrsa.o test_encdec test_encdec.o signal signal.o test_bindwidth_server test_bindwidth_server.o test_bindwidth_client test_bindwidth_client.o \
	test_interrupt test_interrupt.o test_soap_send test_soap_send.o test_local_server test_local_serve.o test_local_client test_local_client.o https_get https_get.o
