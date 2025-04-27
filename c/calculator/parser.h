#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void skip_whitespace(const char** input)
{
	while (isspace(**input))
	{
		(*input)++;
	}
}

int parse_number(const char** input, int* error)
{
	skip_whitespace(input);
	int num = 0;
	int sign = 1;

	if (**input == '-')
	{
		sign = -1;
		(*input)++;
	}
	else if (**input == '+')
	{
		(*input)++;
	}

	if (!isdigit(**input))
	{
		fprintf(stderr, "Expected number\n");
		*error = ERROR_PARSE_ERROR;
		return 0;
	}

	while (isdigit(**input))
	{
		num = num * 10 + (**input - '0');
		(*input)++;
	}

	return num * sign;
}

int power(int base, int exp, int* error)
{
	if (exp < 0)
	{
		fprintf(stderr, "Negative exponent not supported for integer power\n");
		*error = ERROR_UNSUPPORTED_OPERATION;
		return 0;
	}
	int result = 1;
	for (int i = 0; i < exp; i++)
	{
		result *= base;
	}
	return result;
}

int parse_expr(const char** input, int* error)
{
	int value = parse_term(input, error);

	while (*error == 0)
	{
		skip_whitespace(input);

		if (**input == '+')
		{
			(*input)++;
			value += parse_term(input, error);
		}
		else if (**input == '-')
		{
			(*input)++;
			value -= parse_term(input, error);
		}
		else
		{
			break;
		}
	}

	return value;
}

int parse_term(const char** input, int* error)
{
	int value = parse_factor(input, error);

	while (*error == 0)
	{
		skip_whitespace(input);

		if (**input == '*')
		{
			(*input)++;
			value *= parse_factor(input, error);
		}
		else if (**input == '/')
		{
			(*input)++;
			int divisor = parse_factor(input, error);
			if (divisor == 0)
			{
				*error = ERROR_DIVISION_BY_ZERO;
				return 0;
			}
			value /= divisor;
		}
		else if (**input == '%')
		{
			(*input)++;
			int divisor = parse_factor(input, error);
			if (divisor == 0)
			{
				*error = ERROR_DIVISION_BY_ZERO;
				return 0;
			}
			value %= divisor;
		}
		else if (**input == '&')
		{
			(*input)++;
			value &= parse_factor(input, error);
		}
		else if (**input == '|')
		{
			(*input)++;
			value |= parse_factor(input, error);
		}
		else if (**input == '^')
		{
			(*input)++;
			value ^= parse_factor(input, error);
		}
		else if (**input == '>' && *(*input + 1) == '>')
		{
			(*input) += 2;
			int shift = parse_factor(input, error);
			if (shift < 0 || shift >= 32)
			{
				fprintf(stderr, "Invalid right shift amount: %d\n", shift);
				error = ERROR_UNSUPPORTED_OPERATION;
				return 0;
			}
			value >>= shift;
		}
		else if (**input == '<' && *(*input + 1) == '<')
		{
			(*input) += 2;
			int shift = parse_factor(input, error);
			if (shift < 0 || shift >= 32)
			{
				fprintf(stderr, "Invalid right shift amount: %d\n", shift);
				error = ERROR_UNSUPPORTED_OPERATION;
				return 0;
			}
			value <<= shift;
		}
		else
		{
			break;
		}
	}

	return value;
}

int parse_factor(const char** input, int* error)
{
	skip_whitespace(input);

	int sign = 1;

	if (**input == '-')
	{
		sign = -1;
		(*input)++;
	}
	else if (**input == '+')
	{
		(*input)++;
	}
	else if (**input == '~')
	{
		(*input)++;
		return ~parse_factor(input, error);
	}

	int value = parse_base(input, error);

	skip_whitespace(input);
	if (**input == '*' && *(*input + 1) == '*')
	{
		(*input) += 2;
		int exponent = parse_factor(input, error);
		value = power(value, exponent, error);
	}

	return value * sign;
}

int parse_base(const char** input, int* error)
{
	skip_whitespace(input);

	if (**input == '(')
	{
		(*input)++;
		int value = parse_expr(input, error);
		skip_whitespace(input);

		if (**input != ')')
		{
			*error = ERROR_PARSE_ERROR;
			return 0;
		}

		(*input)++;
		return value;
	}
	else
	{
		return parse_number(input, error);
	}
}