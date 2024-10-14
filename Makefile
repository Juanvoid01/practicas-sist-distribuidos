#SSL_LIBS=-lssl -lcrypto
#SSL_FLAGS=-DWITH_OPENSSL

SSL_LIBS=
SSL_FLAGS=

all: soapC.c client server

soapC.c:
	soapcpp2 -b -c conecta4.h

client:
	gcc $(SSL_FLAGS) -o client client.c soapC.c soapClient.c game.c -lgsoap $(SSL_LIBS) -L$(GSOAP_LIB) -I$(GSOAP_INCLUDE)

server:	
	gcc $(SSL_FLAGS) -o server server.c soapC.c soapServer.c game.c -lgsoap -lpthread $(SSL_LIBS) -L$(GSOAP_LIB) -I$(GSOAP_INCLUDE)


clean:	
	rm client server game.o *.xml *.nsmap *.wsdl *.xsd soapStub.h soapServerLib.* soapH.h soapServer.* soapClientLib.* soapClient.* soapC.*
