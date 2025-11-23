#define _GNU_SOURCE
#include <zstd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <elf.h>

void* read_file(const char* filename, size_t* p_size) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open"); fflush(stderr); exit(1);
    }
    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) {
        perror("lseek"); fflush(stderr); exit(1);
    }
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek"); fflush(stderr); exit(1);
    }
    void* buffer = malloc((size_t)size);
    if (buffer == NULL) {
        perror("malloc"); fflush(stderr); exit(1);
    }
    if (read(fd, buffer, size) != size) {
        perror("read"); fflush(stderr); exit(1);
    }
    if (close(fd) < 0) {
        perror("close"); fflush(stderr); exit(1);
    }
    if (p_size != NULL) { 
        *p_size = size;
    }
    return buffer;
}
void write_file(const char* filename, const void* buffer, size_t size) {
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC);
    if (fd < 0) {
        perror("open"); fflush(stderr); exit(1);
    }
    if (write(fd, buffer, size) != (ssize_t)size) {
        perror("write"); fflush(stderr); exit(1);
    }
    if (close(fd) < 0) {
        perror("close"); fflush(stderr); exit(1);
    }
}
int create_mem_file(const void* buffer, size_t size) {
    int fd = memfd_create("memfile", 0);
    if (fd < 0) {
        perror("memfd_create"); fflush(stderr); exit(1);
    }
    if (write(fd, buffer, size) != (ssize_t)size) {
        perror("write"); fflush(stderr); exit(1);
    }
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek"); fflush(stderr); exit(1);
    }
    return fd;
}
void* decompress(const void* buffer, size_t size, size_t* p_decompressed_size) {
    unsigned long long decompressed_size = ZSTD_getFrameContentSize(buffer, size);
    if (ZSTD_isError(decompressed_size)) {
        fprintf(stderr, "ZSTD_getFrameContentSize: %s\n", ZSTD_getErrorName(decompressed_size)); fflush(stderr); exit(1);
    }
    void* decompressed_buffer = malloc(decompressed_size);
    if (decompressed_buffer == NULL) {
        perror("malloc"); fflush(stderr); exit(1);
    }
    size_t zstd_error_code = ZSTD_decompress(decompressed_buffer, decompressed_size, buffer, size);
    if (ZSTD_isError(zstd_error_code)) {
        fprintf(stderr, "ZSTD_decompress: %s\n", ZSTD_getErrorName(zstd_error_code)); fflush(stderr); exit(1);
    }
    if (p_decompressed_size != NULL) {
         *p_decompressed_size = decompressed_size;
    }
    return decompressed_buffer;
}
char* read_elf_section(const char* buffer, size_t size, const char* section_name, size_t* p_section_size) {
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)buffer;

    if (size < SELFMAG || memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        fprintf(stderr, "%s\n", "read_elf_section: Not an ELF file."); fflush(stderr); exit(1);
    }
    
    Elf64_Shdr *shdrs = (Elf64_Shdr *)(buffer + ehdr->e_shoff);
    Elf64_Shdr *shstr = &shdrs[ehdr->e_shstrndx];
    const char *shstrtab = buffer + shstr->sh_offset;
    
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (strcmp(shstrtab + shdrs[i].sh_name, section_name) == 0) {
            if (p_section_size != NULL) {
                *p_section_size = shdrs[i].sh_size;
            }
            char* section_buffer = malloc(shdrs[i].sh_size);
            memcpy(section_buffer, buffer + shdrs[i].sh_offset, shdrs[i].sh_size);
            return section_buffer;
        }
    }
    
    fprintf(stderr, "%s\n", "read_elf_section: Section not found."); fflush(stderr); exit(1);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    size_t size;
    char* module = read_file("/usr/lib/modules/6.16.5-arch1-1/vmlinuz", &size);
    
    //size_t size;
    //char* module = decompress(compressed_module, compressed_size, &size);

    size_t section_size;
    char* section = read_elf_section(module, size, ".modinfo", &section_size);
    
    char* curstr = section;
    while (curstr < section + section_size) {
        printf("%s\n", curstr);
        curstr += strlen(curstr) + 1;
    }

    int module_fd = create_mem_file(module, size);

    char* args = NULL;
    if (syscall(SYS_finit_module, module_fd, &args, 0) != 0) {
        perror("finit_module"); fflush(stderr); exit(1);
    }

    return 0;
}