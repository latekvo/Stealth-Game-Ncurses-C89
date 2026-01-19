# Based on https://github.com/latekvo/tinyscript/blob/main/Makefile
TARGET=app
BUILD_DIR=build/
FLAGS=-ansi -g
LINKS=-lncurses -lm

OBJECTS=$(addprefix $(BUILD_DIR),$(subst .c,.o,$(wildcard *.c)))

default: $(TARGET) 
.PHONY: clean test 

$(TARGET): $(OBJECTS) 
	gcc $(FLAGS) $(LINKS) -o $@ $^ 

$(BUILD_DIR)%.o: %.c
	mkdir -p $(BUILD_DIR)
	gcc $(FLAGS) -c -o $@ $(notdir $(subst .o,.c,$@))

clean:
	rm -r $(BUILD_DIR) $(TARGET)

