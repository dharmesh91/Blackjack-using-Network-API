CC = gcc

CFLAGS = -g -Wall
LDFLAGS = $(CFLAGS) -pthread
TARGET_SRV = masterserver
TARGET_CLT = client
SOURCES_SRV = $(TARGET_SRV).c errexit.c conversions.c
SOURCES_CLT = $(TARGET_CLT).c errexit.c conversions.c

all: $(TARGET_SRV) $(TARGET_CLT)

$(TARGET_SRV): $(SOURCES_SRV)
	$(CC) $(LDFLAGS) -o $(TARGET_SRV) $(SOURCES_SRV)

$(TARGET_CLT): $(SOURCES_CLT)
	$(CC) $(CFLAGS) -o $(TARGET_CLT) $(SOURCES_CLT)

clean:
	$(RM) $(TARGET_SRV) $(TARGET_CLT)
