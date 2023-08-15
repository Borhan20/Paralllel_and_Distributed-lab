# Compiler
CC := g++
# Compiler flags
CFLAGS := -Wall -std=c++11
# Source files
SERVER_SRC := server.cpp
CLIENT_SRC := client.cpp
# Output files
SERVER_EXEC := server
CLIENT_EXEC := client

# Default target
all: $(SERVER_EXEC) $(CLIENT_EXEC)

# Target for the server executable
$(SERVER_EXEC): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $<

# Target for the client executable
$(CLIENT_EXEC): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $<

# Clean all generated files
clean:
	rm -f $(SERVER_EXEC) $(CLIENT_EXEC)

