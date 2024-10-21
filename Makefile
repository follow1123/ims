CC = gcc

SRC = src/main.c
TEST_SRC = tests/main_test.c

EXEC = ims.exe
TEST_EXEC = test_ims.exe

build:
	$(CC) $(SRC) -s -o $(EXEC) -luser32 -lgdi32

test:
	$(CC) $(TEST_SRC) -o $(TEST_EXEC) -luser32 -lgdi32 & test_ims.exe

clean:
	cmd /c del $(EXEC) $(TEST_EXEC)

run: build
	$(EXEC)

.PHONY: build clean
