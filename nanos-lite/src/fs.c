#include <fs.h>


typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);


extern size_t init_fb();
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern size_t serial_write(const void *buf, size_t offset, size_t len);
extern size_t events_read(void *buf, size_t offset, size_t len);
extern size_t dispinfo_read(void *buf, size_t offset, size_t len);
extern size_t fb_write(const void *buf, size_t offset, size_t len);
extern size_t sbctl_read(void *buf, size_t offset, size_t len);
extern size_t sbctl_write(const void *buf, size_t offset, size_t len);
extern size_t sb_write(const void *buf, size_t offset, size_t len);
typedef struct {
    char *name;
    size_t size;
    size_t disk_offset;
    size_t open_offset;
    ReadFn read;
    WriteFn write;
} Finfo;

enum { FD_STDIN,
       FD_STDOUT,
       FD_STDERR,
       FD_EVENTS,
       FD_DISINFO,
       FD_FBUF,
       FD_SB,
       FD_SBCTL,
       FD_FB };

size_t invalid_read(void *buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
        [FD_STDIN] = {"stdin", 0, 0, 0, invalid_read, invalid_write},
        [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, serial_write},
        [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, serial_write},
        [FD_EVENTS] = {"/dev/events", 0, 0, 0, events_read, invalid_write},
        [FD_DISINFO] = {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
        [FD_FBUF] = {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
        [FD_SB] = {"/dev/sb", 0, 0, 0, invalid_read, sb_write},
        [FD_SBCTL] = {"/dev/sbctl", 0, 0, 0, sbctl_read, sbctl_write},
#include "files.h"
};


void init_fs() {
    file_table[FD_FBUF].size = init_fb();
}


int fs_close(int fd) {
    return 0;
}

size_t fs_read(int fd, void *buf, size_t len) {
    if (file_table[fd].read != NULL) {
        assert(0);
        return file_table[fd].read(buf, 0, len);
    }
    if (len == 0) {
        assert(0);
        return 0;
    }
    if (file_table[fd].open_offset == 0)
        file_table[fd].open_offset = file_table[fd].disk_offset;
    if (file_table[fd].open_offset + len > file_table[fd].disk_offset + file_table[fd].size) {
        size_t ret = ramdisk_read(buf, file_table[fd].open_offset, file_table[fd].disk_offset + file_table[fd].size - file_table[fd].open_offset);
        file_table[fd].open_offset = file_table[fd].disk_offset + file_table[fd].size;
        assert(0);
        return ret;
    }
    size_t ret = ramdisk_read(buf, file_table[fd].open_offset, len);
    file_table[fd].open_offset += len;
    assert(ret);
    assert(0);
    return ret;
}

size_t fs_write(int fd, const void *buf, size_t len) {
    if (file_table[fd].write != NULL)
        return file_table[fd].write(buf, file_table[fd].open_offset, len);
    if (len == 0 || file_table[fd].open_offset > file_table[fd].size + file_table[fd].disk_offset || file_table[fd].open_offset < file_table[fd].disk_offset)
        return 0;
    if (file_table[fd].open_offset == 0)
        file_table[fd].open_offset = file_table[fd].disk_offset;
    if (file_table[fd].open_offset + len > file_table[fd].disk_offset + file_table[fd].size)
        len = file_table[fd].disk_offset + file_table[fd].size - file_table[fd].open_offset;
    size_t ret = ramdisk_write(buf, file_table[fd].open_offset, len);
    file_table[fd].open_offset += len;
    return ret;
}


/// @brief rewind your file pointer littel bug remain
/// @param fd
/// @param offset
/// @param whence
/// @return
size_t fs_lseek(int fd, size_t offset, int whence) {
    switch (whence) {
        case SEEK_SET: {
            file_table[fd].open_offset = file_table[fd].disk_offset + offset;
            break;
        }
        case SEEK_CUR: {
            file_table[fd].open_offset = file_table[fd].open_offset + offset;
            break;
        }
        case SEEK_END: {
            file_table[fd].open_offset = file_table[fd].disk_offset + file_table[fd].size + offset;
            break;
        }
        default: {
            panic("wrong whence please check your program!");
        }
    }
    if (file_table[fd].disk_offset != 0) {
        if (file_table[fd].open_offset > file_table[fd].disk_offset + file_table[fd].size)
            file_table[fd].open_offset = file_table[fd].size + file_table[fd].disk_offset;
        if (file_table[fd].open_offset < file_table[fd].disk_offset)
            file_table[fd].open_offset = file_table[fd].disk_offset;
    }
    return file_table[fd].open_offset;
}

int fs_open(const char *pathname, int flags, int mode) {
    int i;
    for (i = 0; file_table[i].name != NULL; ++i)
        if (strcmp(file_table[i].name, pathname) == 0) {
            fs_lseek(i, 0, SEEK_SET);
            return i;
        }
    printf("%s %s\n", file_table[i - 1].name, pathname);
    panic("no such file please check files.h!");
}