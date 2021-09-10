#include <stdio.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <unistd.h>
#include <ctype.h>
#include <regex.h>
#include <string.h>

char* get_ip_addr(){
	// Function finds available network interfaces and their addresses
	// It then loops through them and returns the final interfaces address
	// This is done as the first interface is the loopback and alway has
	// the address 127.0.0.1 so would not be useful to output

	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char *addr;
	getifaddrs (&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
	        if (ifa->ifa_addr && ifa->ifa_addr->sa_family==AF_INET) {
	            sa = (struct sockaddr_in *) ifa->ifa_addr;
	            addr = inet_ntoa(sa->sin_addr);
	            //printf("Interface: %s | Address: %s", ifa->ifa_name, addr);
	        }
	}

	freeifaddrs(ifap);
	return addr;
}


char* read_temp(){
	// Function returns the CPU temperature as found in the sys directory
	// Note: this may not exist on all systems however works for raspberry pi
	FILE *tempFile;
	double T;
	tempFile = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	if (tempFile == NULL){
		printf("Error, tempfile not found");
	}
	fscanf(tempFile, "%lf", &T);
	T /= 1000;
	fclose(tempFile);
	char* temp_string = (char *)malloc(sizeof(char) * 7);
	sprintf(temp_string, "%6.3f°C", T);
	return temp_string;
}

char* get_uptime(){
	// Gets the system uptime and converts it into a more human readable format
	struct sysinfo s_info;
	const int error = sysinfo(&s_info);
	if(error != 0){
		printf("code error = %d\n", error);
	}
	long t_seconds = s_info.uptime;
	const int hours = t_seconds / 3600;
	t_seconds -= 3600 * hours;
	const int minutes = t_seconds / 60;
	t_seconds -= 60 * minutes;
	const char* pattern = "%dH:%dM:%dS";
	char* return_string = (char*)malloc(sizeof(char)*20);
	sprintf(return_string,pattern, hours, minutes, t_seconds);
	return return_string;
}

char* get_mem_value_from_line(char* line, ssize_t strlen){
	// Takes in a line from proc/meminfo and extracts the numeric value
	char* value = malloc(sizeof(char) * 10);
	int end_index = 0;
	for (int i = 0; i < strlen; i++){
		if(isdigit(line[i])){
			value[end_index] = line[i];
			end_index += 1;
		}
	}
	value[end_index] = '\0';
	return value;
}

char* get_mem_usage(){
	// Returns information on used and available memory on the system
	FILE* memfile = fopen("/proc/meminfo", "r");
	if (memfile == NULL){ return "Memory File Read Error"; }
	char* line;
	ssize_t read;
	ssize_t total_read = 0;
	size_t len = 0;

	int used, total;

	// total memory is line 1
	read = getline(&line, &len, memfile);
	char* total_line = get_mem_value_from_line(line, read);
	total_read = read;
	sscanf(total_line, "%d", &total);

	// available memory is on line 3
	getline(&line, &len, memfile);
	read = getline(&line, &len, memfile);
	char* used_line = get_mem_value_from_line(line, read);
	total_read += read;
	sscanf(used_line, "%d", &used);

	int free = total - used;
	free /= 1024;
	total /= 1024;

	fclose(memfile);

	char* formatted_line = malloc(sizeof(char) * total_read);
	sprintf(formatted_line, "%dM/%dM", free, total);

	return formatted_line;
}

int print(const int side, char* title, char* msg, const char* colour){
	// Prints a formatted line in the terminal with colour options and offset
	// if required
	const int logo_size = 10;
	const int offset = 3;
	const char* esc = "\033";

	printf("\r");
	if (side) {
		// print info (right side)

		for(int i = 0; i < offset + logo_size; i++){
			printf(" ");
		}
	}
	printf("%s%s%s%s%s%s", esc, colour, title, esc, "[0m", msg);
	if (!side) {printf("\n");}
}

int check_line(const char* line, const char* match){
	// Use REGEX to find expression in line
	// Variable to store initial regex()
  regex_t regex;

  // Variable for return type
  int value;

  // Creation of regEx
  value = regcomp(&regex, match, 0);

  // Comparing pattern "Geeks"
  // with string in reg
  value = regexec(&regex, line, 0, NULL, 0);

  if (value == 0) { return 1; }
  return 0;
}

char* get_distro(){
	// Function reads the /etc/*-release file and uses the ID line to determine OS
	FILE* fp = fopen("/etc/os-release", "r");
	if (fp == NULL){
		printf("File Read Error\n");
		return "release file Read Error"; }
	char* line;
	ssize_t read;
	size_t len = 0;
	char* distro;

	// Loop through lines until ID= is found
	int found = 0;
	while (!found) {
		read = getline(&line, &len, fp);
		if (check_line(line, "^ID=")) {
			found = 1;
		}
	}

	fclose(fp);
	return line;

}

char** get_logo(){
	FILE* fp;
	char* distro = get_distro();
	//printf("Distro: %s\n", distro);
	
	if (strcmp(distro, "ID=arch\n") == 0) {
		//printf("Distro: Arch\n");
		fp = fopen("distros/arch.txt", "r");
	} else if (strcmp(distro, "ID=raspbian\n") == 0){
		//printf("Distro: Raspbian\n");
		fp = fopen("distros/raspbian.txt", "r");
	}

	if (fp == NULL) {
		printf("Error opening distro file\n");
		return NULL;
	}

	char* line;
	ssize_t read;
	size_t len = 0;
	char** logo = malloc(sizeof(char*) * 5);
	for (int i = 0; i<5; i++){
		read = getline(&line, &len, fp);
		line[read-1] = '\0';
		//printf("\033%s\n", line);
		logo[i] = (char*)malloc(sizeof(char) * read + 1);
		strcpy(logo[i], line);
	}
	return logo;
}
