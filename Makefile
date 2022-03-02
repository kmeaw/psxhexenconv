all: win64.exe lin64

.PHONY: all clean

clean:
	rm -f win64.exe lin64

win64.exe: convert.c
	zig cc -Os -static -target x86_64-windows-gnu $< -o $@

lin64: convert.c
	zig cc -Os -static -target x86_64-linux-musl $< -o $@
