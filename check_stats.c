#include <stdio.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <unistd.h>

void get_ip_addr(){
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char *addr;
	getifaddrs (&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
	        if (ifa->ifa_addr && ifa->ifa_addr->sa_family==AF_INET) {
	            sa = (struct sockaddr_in *) ifa->ifa_addr;
	            addr = inet_ntoa(sa->sin_addr);
	            printf("Interface: %s | Address: %s", ifa->ifa_name, addr);
	        }
	}

	freeifaddrs(ifap);
}


char* read_temp(){
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
	sprintf(temp_string, "%6.3f'C", T);
	return temp_string;
}

char* get_uptime(){
	struct sysinfo s_info;
	int error = sysinfo(&s_info);
	if(error != 0){
		printf("code error = %d\n", error);
	}
	long t_seconds = s_info.uptime;
	int hours = t_seconds / 3600;
	t_seconds -= 3600 * hours;
	int minutes = t_seconds / 60;
	t_seconds -= 60 * minutes;
	char* pattern = "%dH:%dM:%dS";
	char* return_string = (char*)malloc(sizeof(char)*20);
	sprintf(return_string,pattern, hours, minutes, t_seconds);
	return return_string;
}

int print(const int side, char* title, char* msg, const char* colour){
	const int offset = 3;
	const int ascii_size = 6;
	const char* esc = "\033";

	if (side) {
		// print info (right side)
		for(int i = 0; i < offset + ascii_size; i++){
			printf(" ");
		}
	}
	printf("%s%s%s%s%s%s", esc, colour, title, esc, "[0m", msg);
	if (side) {printf("\n");}
}

int main(){

	const char* RED = "[0;31m";
	const char* GREEN = "[1;32m";
	char* temp = read_temp();
	char* uptime = get_uptime();

	// For some reason attempting to get hostname via getenv("HOSTNAME")
	// returns null despite $HOSTNAME being a valid env variable
	// This function is also not in linux/unistd.h hence the second include
	char hostname[255 + 1];
	gethostname(hostname, 255 + 1);

	char host[255];
	sprintf(host, "%s@%s", getenv("USER"), hostname);

	print(1, host, "", GREEN);
	print(1, "Current uptime: ", uptime, RED);
	print(1, "CPU Temp: ", temp, RED);

	//get_ip_addr();

	free(uptime);
	return 0;
}
