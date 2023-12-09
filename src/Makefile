CC := clang
CFLAGS := -g 
LIBS_SERVER := -lpthread -lncurses
all: pong server client

clean:
	rm -f server client

pong: pong.c util.c util.h scheduler.c scheduler.h
	$(CC) $(CFLAGS) -o pong pong.c util.c scheduler.c -lncurses

server: server.c message.h message.c socket.h util.c util.h scheduler.c scheduler.h
	$(CC) $(CFLAGS) -o server server.c message.c util.c scheduler.c $(LIBS_SERVER)

client: client.c message.h message.c util.c util.h
	$(CC) $(CFLAGS) -o client client.c message.c util.c -lncurses

zip:
	@echo "Generating pong.zip file to submit to Gradescope..."
	@zip -q -r pong.zip . -x .git/\* .vscode/\* .clang-format .gitignore pong
	@echo "Done. Please upload pong.zip to Gradescope."

format:
	@echo "Reformatting source code."
	@clang-format -i --style=file $(wildcard *.c) $(wildcard *.h)
	@echo "Done."

.PHONY: all clean zip format
