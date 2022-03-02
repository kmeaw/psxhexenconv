#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define READ(var) do {\
  if (fread(&var, sizeof(var), 1, f) != 1) { \
    perror("short read: " #var); \
    abort(); \
  } \
} while(0)

#define WRITE(var) do {\
  if (fwrite(&var, sizeof(var), 1, f) != 1) { \
    perror("short write: " #var); \
    abort(); \
  } \
} while(0)

void patch_vertexes(FILE *f, uint32_t pos, uint32_t size) {
    uint32_t i;
    for (i = 0; i < size; i += sizeof(uint32_t) * 2) {
        uint32_t x32, y32;
        uint16_t x16, y16;
        if(fseek(f, pos + i, SEEK_SET)) {
            perror("cannot seek to vertexes");
            abort();
        }
        READ(x32);
        READ(y32);
        x16 = x32 / 65536;
        y16 = y32 / 65536;
        if(fseek(f, pos + i / 2, SEEK_SET)) {
            perror("cannot seek to vertexes");
            abort();
        }
        WRITE(x16);
        WRITE(y16);
    }
}

void convert(const char *filename) {
    uint32_t i;
    FILE *f = fopen(filename, "r+b");
    if (!f) {
        perror(filename);
        abort();
    }

    char sig[5] = { [4] = 0 };
    if (fread(sig, 4, 1, f) != 1) {
        perror("short read: signature");
        abort();
    }

    if (!strcmp(sig, "IWAD") && !strcmp(sig, "PWAD")) {
        fprintf(stderr,"unexpected magic: %s\n", sig);
        abort();
    }

    uint32_t lumps, dir_offset;
    READ(lumps);
    READ(dir_offset);

    if (fseek(f, dir_offset, SEEK_SET)) {
        perror("cannot seek to dir_offset");
        abort();
    }

    for(i=0; i<lumps; i++) {
        char name[9] = { [8] = 0 };
        uint32_t pos, size;
        long loc = ftell(f);
        READ(pos);
        READ(size);
        if (fread(name, 8, 1, f) != 1) {
            perror("cannot read lump name");
            abort();
        }
        if (!strcmp(name, "VERTEXES")) {
            patch_vertexes(f, pos, size);
            if(fseek(f, loc, SEEK_SET)) {
                perror("cannot seek back");
                abort();
            }
            size /= 2;
            WRITE(pos);
            WRITE(size);
        }
    }

    fclose(f);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <MAP01.WAD> [<MAP02.WAD> …]\n", argv[0]);
        return 1;
    }

    int i;
    for (i = 1; i < argc; i++) {
        convert(argv[i]);
    }
}
