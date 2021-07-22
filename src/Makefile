BIN:=msqt
.PHONY:all clean

all: $(BIN)

$(BIN): main.c
	$(CC) -o $@ $^ -luci -lmosquitto

clean: 
	rm -r $(BIN)