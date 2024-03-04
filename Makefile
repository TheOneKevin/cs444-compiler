BUILD_DIR = ./build1

all:
	@echo "Building joosc"
	@mkdir -p $(BUILD_DIR)
	make -f Makefile1 -j$(nproc) joosc BUILD_DIR=$(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR) && rm -f joosc

.PHONY: all clean
