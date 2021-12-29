#include<stdio.h>
#include<string.h>

#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#include<sys/mman.h>

#include<elf.h>

unsigned char* find_text(void* data, size_t *size);
size_t get_file_size(int fd);
int main(int argc, char *argv[]){
	unsigned int i;
	int fd;

	if(argc != 2){
		printf("Usage : %s elf\n", argv[0]);
		return -1;
	}

	if( (fd = open(argv[1], O_RDONLY)) < 0){
		perror("open");
		return -1;
	}

	size_t section_size = 0;
	size_t size = get_file_size(fd);
	if(!size){
		puts("file is empty!");
		close(fd);
		return -1;
	}

	unsigned char *text = NULL;
	void *file = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if(!file){
		perror("mmap");
		close(fd);
		munmap(file, size);
		return -1;
	}
	text = (unsigned char *)find_text(file, &section_size);
	if(!text){
		close(fd);
		munmap(file, size);
		return -1;
	}
	for(i = 0;i < section_size; i++)
		printf("\\x%02x", text[i]);
	puts("");

	close(fd);
	munmap(file, size);

	return 0;
}
size_t get_file_size(int fd){
	struct stat st;
	if(fstat(fd, &st) < 0){
		perror("fstat");
		return -1;
	}	
	return st.st_size;
}
unsigned char* find_text(void *data, size_t *size){
	unsigned int i;
	Elf32_Ehdr *head = (Elf32_Ehdr *)data;
	Elf32_Shdr *section = (Elf32_Shdr *)((char *)data + head -> e_shoff);
	unsigned char *str = (unsigned char *)data + ((section + head -> e_shstrndx) -> sh_offset);
	if(head -> e_ident[EI_CLASS] != ELFCLASS32){
		fprintf(stderr, "this program only support 32 bit executables!\n");
		return NULL;
	}

	for(i = 0; i < head -> e_shnum; i++){
		if(!strcmp((char *)str + section -> sh_name, ".text")){
			*size = section -> sh_size;
			return (unsigned char *)data + section -> sh_offset;
		}
		++section;
	}
	fprintf(stderr, ".text section wasn't found! be sure to supply a non-stripped binary\n");
	return NULL; 
}
