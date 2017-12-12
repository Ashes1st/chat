/*
** showip.c -- выводит IP адреса заданного в командной строке хоста 
*/
#include <stdio.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <stdlib.h> 

char* itoa(int i, char b[]){
	char const digit[] = "0123456789";
	char* p = b;
	if(i<0){
		*p++ = '-';
		i *= -1;
	}
	int shifter = i;
	do{ //Move to where representation ends
		++p;
		shifter = shifter/10;
	}while(shifter);
	*p = '\0';
	do{ //Move back, inserting digits as u go
		*--p = digit[i%10];
		i = i/10;
	}while(i);
	return b;
}

int main(int argc, char *argv[])
{

	char buffer[] = "-";
	char *nick = "ashes: /history";
	char ll[100];
	memset(ll, 0, sizeof ll);
	char *k = "/history";
	int a = 123;
	
	char ch[50];
	int LEN_HIS = 10;
	char history[LEN_HIS][25];
	int tec_pos_his = 0;
	int krug = 0;


	char *e = strcat(ll, buffer);


	printf("%s   %d  ", e, strcmp(e, buffer));

	//itoa(a,ch);

/*
while(1)
{
	scanf("%s", buffer);
	printf("%s\n", buffer);
	if(strcmp(buffer, "/history") == 0)
	{
		if(krug == 0)
		{
			for(int k = 0; k < tec_pos_his; k++)
			{
				printf("%d %s\n",k, history[k]);
			}
		}
		else
		{
			for(int k = tec_pos_his; k < 25; k++)
			{
				printf("%d %s\n",k, history[k]);
			}
			for(int k = 0; k < tec_pos_his; k++)
			{
				printf("%d %s\n",k, history[k]);
			}
		}
		printf("\nend history coming\n");
	}
	else
	{
		memmove(history[tec_pos_his], buffer, 25);
		tec_pos_his++;
		if(tec_pos_his == LEN_HIS)
		{
			tec_pos_his = 0;
			krug = 1;
		}
	}
}
*/	

	//printf("%s", ch);


/*
	for(int i = 0; i < 15; i++)
		printf("%d ", i);

	printf("\n");

	for(int *i = *nick; i < 15; i++)
		printf("%c ", *i);
*/
	/*
	memset(all, 0, sizeof all);
	char *ll = strcat(all, nick);
	ll = strcat(ll, nick);
	scanf("%s", buffer);
	char *e = strcat(ll, buffer);
	//strcat(all, buffer);
	printf("%s - strcat |||||||  %s", strcat(ll, buffer), e);
*/
	/*
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN]; 

	printf("%d\n", (int)strlen(argv[1]));

	if (argc != 2){
		fprintf(stderr,"usage: showip hostname\n");
		return 1; 
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) { 
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		
		return 2; 
	}
	
	printf("IP addresses for %s:\n\n", argv[1]); 

	for(p = res; p != NULL; p = p->ai_next) {
		
		void *addr; char *ipver;
		// получить,
		// в IPv4 и IPv6 поля разные: 
		if (p->ai_family == AF_INET) {// IPv4
			
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4"; 
		
		} else {// IPv6

			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr; addr = &(ipv6->sin6_addr);
			ipver = "IPv6";

		}
		
		// перевести IP в строку и распечатать:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf(" %s: %s\n", ipver, ipstr); 
	
	}

	freeaddrinfo(res); // освободить связанный список 
	*/
	return 0;
}





