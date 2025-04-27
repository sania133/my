#include "int_128.h"

#include "utils.h"

int128_t parse_number(const char *str)
{
	int128_t result = { 0, 0 };
	int base = 10;

	if (my_strncmp(str, "0x", 2) == 0 || my_strncmp(str, "0X", 2) == 0)
	{
		base = 16;
		str += 2;
	}
	else if (my_strncmp(str, "0b", 2) == 0 || my_strncmp(str, "0B", 2) == 0)
	{
		base = 2;
		str += 2;
	}
	else if (*str == '0' && *(str + 1) != '\0')
	{
		base = 8;
		str++;
	}

	int is_negative = 0;
	if (*str == '-')
	{
		is_negative = 1;
		str++;
	}
	else if (*str == '+')
	{
		str++;
	}

	while (*str)
	{
		int digit =
			(*str >= '0' && *str <= '9')   ? (*str - '0')
			: (*str >= 'a' && *str <= 'f') ? (*str - 'a' + 10)
			: (*str >= 'A' && *str <= 'F')
				? (*str - 'A' + 10)
				: -1;

		if (digit < 0 || digit >= base)
		{
			return (int128_t){ 0, 0 };
		}

		uint64_t new_low = result.low * base + digit;
		uint64_t carry = (new_low < result.low) ? 1 : 0;

		result.low = new_low;
		result.high = result.high * base + carry;
		str++;
	}

	if (is_negative)
	{
		result.high = ~result.high + (result.low == 0);
		result.low = ~result.low + 1;
	}

	return result;
}

void number_to_string(int128_t value, int base, char *buffer, int uppercase)
{
	char digits[] = "0123456789abcdef";
	if (uppercase)
	{
		for (int i = 10; i < 16; i++)
		{
			digits[i] = (char)(digits[i] - 'a' + 'A');
		}
	}

	char temp_buffer[128];
	int i = 0;

	int is_negative = 0;
	if ((int64_t)value.high < 0)
	{
		if (base == 10)
		{
			is_negative = 1;

			value.high = ~value.high + (value.low == 0);
			value.low = ~value.low + 1;
		}
	}

	do
	{
		int digit = value.low % base;
		uint64_t carry = value.high % base;
		temp_buffer[i++] = digits[digit];
		value.low /= base;

		value.high = (value.high - carry) / base;
	} while (value.low > 0 || value.high > 0);

	if (is_negative)
	{
		temp_buffer[i++] = '-';
	}

	for (int j = 0; j < i; j++)
	{
		buffer[j] = temp_buffer[i - j - 1];
	}
	buffer[i] = '\0';
}
