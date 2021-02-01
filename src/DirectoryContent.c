/*
 * directory_content.c
 *
 *  Created on: 31 Jan 2021
 *      Author: JR
 */
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <openssl/md5.h>

#if defined(_WIN32)
#define FILESYSTEM_SEPARATOR  "\\"
#else
#define FILESYSTEM_SEPARATOR  "/"
#endif

#define BUFFER_HASHING 1


void search_directory(const char* parent_location, const char* relative_location);

char* get_md5_checksum(char* file_location);

int main(int argc, char** argv){
	printf("Directorio de busqueda: %s\n", argv[1]);
	search_directory(argv[1], "");
	return 0;
}

void search_directory(const char* parent_location, const char* relative_location){
	int size = strlen(parent_location) + strlen(relative_location);
	char* full_location = (char*) malloc(size * sizeof(char*));
	sprintf(full_location, "%s%s", parent_location, relative_location);

	struct dirent* directory;
	DIR* dir_pointer = opendir(full_location);
	free(full_location);

	if(!dir_pointer){
		printf("No se pudo abrir directorio %s", full_location);
		return;
	}


	do{
		directory = readdir(dir_pointer);
		if(directory != NULL){
			if(strcmp(".", directory->d_name) && strcmp("..", directory->d_name)){

				char* child_location = (char*) malloc((1 + strlen(relative_location) + strlen(directory->d_name)) * sizeof(char*));
				sprintf(child_location, "%s%s%s", relative_location, FILESYSTEM_SEPARATOR, directory->d_name);

				char* subdirectory_string = (char*) malloc((strlen(parent_location) + strlen(child_location)) * sizeof(char*));
				sprintf(subdirectory_string, "%s%s", parent_location, child_location);

				DIR* subdirectory_pointer = opendir(subdirectory_string);
				if(subdirectory_pointer){
					free(subdirectory_string);
					closedir(subdirectory_pointer);
					printf("Directorio: %s\n", child_location);
					search_directory(parent_location, child_location);
				}
				else{
					struct stat file_status;
					stat (subdirectory_string, &file_status);

					const int buffer_date = 60;
					char date_time[buffer_date];
					strftime(date_time, buffer_date, "%d/%m/%Y %I:%M:%S %p", localtime(&file_status.st_mtime));

					char* digest = get_md5_checksum(subdirectory_string);
					free(subdirectory_string);

					printf("Archivo: %s | Fecha de modificación: %s | MD5 : %s\n", child_location, date_time, digest);


				}
				free(child_location);
			}
		}

	}while(directory != NULL);

	closedir(dir_pointer);

}

char* get_md5_checksum(char* file_location){
	FILE* file_stream = fopen(file_location, "rb");
	if(file_stream == NULL){
		return NULL;
	}
	else{

		unsigned char md5_array [MD5_DIGEST_LENGTH];
		MD5_CTX context;
		unsigned char buffer[BUFFER_HASHING*2];

		MD5_Init(&context);


		int bytes = fread(buffer, 1, BUFFER_HASHING, file_stream);
		do{
			MD5_Update(&context, buffer, bytes);
			bytes = fread(buffer, 1, BUFFER_HASHING, file_stream);
		}while(bytes != 0);

		MD5_Final(md5_array, &context);

		fclose(file_stream);

		char* hex = (char*) malloc (((MD5_DIGEST_LENGTH * 2) + 1) * sizeof(char*));

		for(int i = 0; i < MD5_DIGEST_LENGTH; i++){
			sprintf(hex+(i*2), "%02x", md5_array[i]);
		}
		return hex;

	}
	return NULL;
}







