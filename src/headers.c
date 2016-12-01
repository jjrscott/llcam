#include <stdio.h>
#include <stdlib.h>
extern char **environ;

int main()
{

	char *range = getenv("HTTP_RANGE");
	
	if (range)
	{
		printf("Content-Type: audio/wav\nContent-Range: %s\n\n", range);

		printf("RI");

	}
	else
	{
		printf("Content-Type: audio/wav\n\n");

		char **env;

		for (env = environ; *env; ++env)
		{
			printf("%s\n", *env);
		}
    
    }
}