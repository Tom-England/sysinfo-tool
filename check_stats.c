#include <stdio.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <linux/kernel.h>
#include <linux/unistd.h> 

void get_ip_addr(){
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char *addr;
	getifaddrs (&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
	        if (ifa->ifa_addr && ifa->ifa_addr->sa_family==AF_INET) {
	            sa = (struct sockaddr_in *) ifa->ifa_addr;
	            addr = inet_ntoa(sa->sin_addr);
	            printf("Interface: %s | Address: %s\n", ifa->ifa_name, addr);
	        }
	}

	freeifaddrs(ifap);
}


double read_temp(){
	FILE *tempFile;
	double T;
	tempFile = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	if (tempFile == NULL){
		printf("Error, tempfile not found");
	}
	fscanf(tempFile, "%lf", &T);
	T /= 1000;
	fclose(tempFile);
	return T;
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
	char* pattern = "%dH:%dM:%dS\n";
	char* return_string = (char*)malloc(sizeof(char)*20);
	sprintf(return_string,pattern, hours, minutes, t_seconds);
	return return_string;
}

int main(){
	double temp = read_temp();
	char* uptime = get_uptime();
	printf("Logged in as: %s\n", getenv("USER"));
	printf("Current uptime: %s", uptime);
	printf("CPU Temp: %6.3f'C\n", temp);
	
	get_ip_addr();

	free(uptime);
	return 0;
}
