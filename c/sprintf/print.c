#include "print.h"

#include "int_128.h"
#include "utils.h"

#include <stdio.h>

void print(char *out_buf, const char *format, const char *number)
{
	if (!my_strchr(format, '%'))
	{
		my_strcpy(out_buf, "Invalid format string.");
		return;
	}

	int left_align = 0, zero_pad = 0, show_sign = 0, show_space = 0, show_prefix = 0;
	int field_width = 0;
	char specifier = 0;

	const char *ptr = format;
	while (*ptr)
	{
		if (*ptr == '%')
		{
			ptr++;
			while (*ptr && my_strchr(" -+#0", *ptr))
			{
				switch (*ptr)
				{
				case '-':
					left_align = 1;
					break;
				case '+':
					show_sign = 1;
					break;
				case ' ':
					show_space = 1;
					break;
				case '#':
					show_prefix = 1;
					break;
				case '0':
					zero_pad = 1;
					break;
				}
				ptr++;
			}
			while (*ptr && *ptr >= '0' && *ptr <= '9')
			{
				field_width = field_width * 10 + (*ptr - '0');
				ptr++;
			}
			if (my_strchr("bodxX", *ptr))
			{
				specifier = *ptr;
				break;
			}
			else
			{
				my_strcpy(out_buf, "Unsupported format specifier.");
				return;
			}
		}
		ptr++;
	}

	if (!specifier)
	{
		my_strcpy(out_buf, "Invalid format string.");
		return;
	}

	int128_t value = parse_number(number);
	char number_buffer[128];
	int base = (specifier == 'b') ? 2 : (specifier == 'o') ? 8 : (specifier == 'x' || specifier == 'X') ? 16 : 10;
	int uppercase = (specifier == 'X');
	number_to_string(value, base, number_buffer, uppercase);

	char formatted_number[128] = "";
	if (show_prefix && base != 10)
	{
		my_strcat(formatted_number, (base == 16) ? (uppercase ? "0X" : "0x") : (base == 8) ? "0" : "0b");
	}
	my_strcat(formatted_number, number_buffer);

	int len = my_strlen(formatted_number);
	int padding = (field_width > len) ? field_width - len : 0;

	char temp_out_buf[256] = { 0 };
	if (left_align)
	{
		my_strcpy(temp_out_buf, formatted_number);
		for (int i = len; i < len + padding; i++)
		{
			temp_out_buf[i] = ' ';
		}
	}
	else
	{
		char pad_char = zero_pad ? '0' : ' ';
		for (int i = 0; i < padding; i++)
		{
			temp_out_buf[i] = pad_char;
		}
		my_strcpy(temp_out_buf + padding, formatted_number);
	}

	my_strcpy(out_buf, temp_out_buf);
}
