#include "utils.h"

size_t my_strlen(const char *str)
{
	size_t length = 0;
	while (*str++)
		length++;
	return length;
}

void my_strcpy(char *dest, const char *src)
{
	while (*src)
		*dest++ = *src++;
	*dest = '\0';
}

void my_strcat(char *dest, const char *src)
{
	while (*dest)
		dest++;
	my_strcpy(dest, src);
}

int my_strchr(const char *str, char ch)
{
	while (*str)
	{
		if (*str == ch)
			return 1;
		str++;
	}
	return 0;
}

int my_strncmp(const char *str1, const char *str2, size_t n)
{
	for (size_t i = 0; i < n; i++)
	{
		if (str1[i] != str2[i] || str1[i] == '\0' || str2[i] == '\0')
		{
			return (unsigned char)str1[i] - (unsigned char)str2[i];
		}
	}
	return 0;
}
