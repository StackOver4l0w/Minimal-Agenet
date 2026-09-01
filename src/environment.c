#include "environment.h"
#include "peb.h"
#include "logger.h"

BOOL CompareEnvName(const WCHAR *wide, const CHAR *narrow)
{
	while (*narrow != '\0')
	{
		WCHAR w = *wide;
		CHAR n = *narrow;

		// Convert to uppercase for comparison
		if (w >= L'a' && w <= L'z')
			w -= 32;
		if (n >= 'a' && n <= 'z')
			n -= 32;

		if (w != (WCHAR)n)
		{
			return false;
		}
		wide++;
		narrow++;
	}

	// After name, should be '='
	return *wide == L'=';
}


USIZE GetVariable(const CHAR *name, CHAR *buffer, USIZE bufferSize){
	if(name == NULL || buffer == NULL || bufferSize == 0)
        return 0;

	buffer[0] = '\0';

    PPEB peb = GetCurrentPEB();
	LOG_INFO("PEb is here: %p", peb);
    if(peb == NULL || peb->ProcessParameters == NULL)
        return 0;

    RTL_USER_PROCESS_PARAMETERS_EX *params = ( RTL_USER_PROCESS_PARAMETERS_EX *)peb->ProcessParameters;
	PWCHAR envBlock = params->Environment;

	if (envBlock == NULL)
	{
		LOG_INFO("Environment block is NULL");
		return 0;
	}

    while (*envBlock != L'\0')
	{
		// Check if this is the variable we're looking for
		if (CompareEnvName(envBlock, name))
		{
			// Find the '=' and skip past it
			const WCHAR *value = envBlock;
			while (*value != L'=' && *value != L'\0')
			{
				value++;
			}
			if (*value == L'=')
			{
				LOG_INFO("Found variable '%s' with value starting at: %p", name, value + 1);
				value++; // Skip the '='

				// Copy value to buffer (convert wide to narrow)
				USIZE len = 0;
				while (*value != L'\0' && len < bufferSize - 1)
				{
					// Simple wide to narrow conversion (ASCII only)
					buffer[len++] = (CHAR)*value++;
				}
				buffer[len] = '\0';
				return len;
			}
			LOG_INFO("Variable '%s' not found in this entry", name);
		}

		// Skip to next variable
		while (*envBlock != L'\0')
		{
			envBlock++;
		}
		envBlock++; // Skip the null terminator
	}
	LOG_INFO("Variable '%s' not found in the environment block", name);

	// Variable not found
	return 0;
}

