/*
 * FreeRTOS+CLI V1.0.4
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */
#include "freertos/FreeRTOS.h"

#ifndef COMMAND_INTERPRETER_H
#define COMMAND_INTERPRETER_H

#define configCOMMAND_INT_MAX_OUTPUT_SIZE 450

/* The prototype to which callback functions used to process command line
commands must comply.  pcWriteBuffer is a buffer into which the output from
executing the command can be written, xWriteBufferLen is the length, in bytes of
the pcWriteBuffer buffer, and pcCommandString is the entire string as input by
the user (from which parameters can be extracted).*/
typedef BaseType_t (*pdCOMMAND_LINE_CALLBACK)( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/* The structure that defines command line commands.  A command line command
should be defined by declaring a const structure of this type. */
typedef struct xCOMMAND_LINE_INPUT
{
	const char * const pcCommand;				/* The command that causes pxCommandInterpreter to be executed.  For example "help".  Must be all lower case. */
	const char * const pcHelpString;			/* String that describes how to use the command.  Should start with the command itself, and end with "\r\n".  For example "help: Returns a list of all the commands\r\n". */
	const pdCOMMAND_LINE_CALLBACK pxCommandInterpreter;	/* A pointer to the callback function that will return the output generated by the command. */
	int8_t cExpectedNumberOfParameters;			/* Commands expect a fixed number of parameters, which may be zero. */
} CLI_Command_Definition_t;

/* For backward compatibility. */
#define xCommandLineInput CLI_Command_Definition_t
/* If the application writer needs to place the buffer used by the CLI at a
fixed address then set configAPPLICATION_PROVIDES_cOutputBuffer to 1 in
FreeRTOSConfig.h, then declare an array with the following name and size in 
one of the application files:
	char cOutputBuffer[ configCOMMAND_INT_MAX_OUTPUT_SIZE ];
*/
#ifndef configAPPLICATION_PROVIDES_cOutputBuffer
	#define configAPPLICATION_PROVIDES_cOutputBuffer 0
#endif

typedef struct xCOMMAND_INPUT_LIST
{
	const CLI_Command_Definition_t *pxCommandLineDefinition;
	struct xCOMMAND_INPUT_LIST *pxNext;
} CLI_Definition_List_Item_t;

/*
 * The callback function that is executed when "help" is entered.  This is the
 * only default command that is always present.
 */
static BaseType_t prvHelpCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Return the number of parameters that follow the command name.
 */
static int8_t prvGetNumberOfParameters( const char *pcCommandString );

/* The definition of the "help" command.  This command is always at the front
of the list of registered commands. */
static const CLI_Command_Definition_t xHelpCommand =
{
	"help",
	"\r\nhelp:\r\n Lists all the registered commands\r\n\r\n",
	prvHelpCommand,
	0
};

/* The definition of the list of commands.  Commands that are registered are
added to this list. */
static CLI_Definition_List_Item_t xRegisteredCommands =
{
	&xHelpCommand,	/* The first command in the list is always the help command, defined in this file. */
	NULL			/* The next pointer is initialised to NULL, as there are no other registered commands yet. */
};

/* A buffer into which command outputs can be written is declared here, rather
than in the command console implementation, to allow multiple command consoles
to share the same buffer.  For example, an application may allow access to the
command interpreter by UART and by Ethernet.  Sharing a buffer is done purely
to save RAM.  Note, however, that the command console itself is not re-entrant,
so only one command interpreter interface can be used at any one time.  For that
reason, no attempt at providing mutual exclusion to the cOutputBuffer array is
attempted.

configAPPLICATION_PROVIDES_cOutputBuffer is provided to allow the application
writer to provide their own cOutputBuffer declaration in cases where the
buffer needs to be placed at a fixed address (rather than by the linker). */
#if( configAPPLICATION_PROVIDES_cOutputBuffer == 0 )
	static char cOutputBuffer[ configCOMMAND_INT_MAX_OUTPUT_SIZE ];
#else
	extern char cOutputBuffer[ configCOMMAND_INT_MAX_OUTPUT_SIZE ];
#endif


/*
 * Register the command passed in using the pxCommandToRegister parameter.
 * Registering a command adds the command to the list of commands that are
 * handled by the command interpreter.  Once a command has been registered it
 * can be executed from the command line.
 */
BaseType_t FreeRTOS_CLIRegisterCommand( const CLI_Command_Definition_t * const pxCommandToRegister );

/*
 * Runs the command interpreter for the command string "pcCommandInput".  Any
 * output generated by running the command will be placed into pcWriteBuffer.
 * xWriteBufferLen must indicate the size, in bytes, of the buffer pointed to
 * by pcWriteBuffer.
 *
 * FreeRTOS_CLIProcessCommand should be called repeatedly until it returns pdFALSE.
 *
 * pcCmdIntProcessCommand is not reentrant.  It must not be called from more
 * than one task - or at least - by more than one task at a time.
 */
BaseType_t FreeRTOS_CLIProcessCommand( const char * const pcCommandInput, char * pcWriteBuffer, size_t xWriteBufferLen  );

/*-----------------------------------------------------------*/

/*
 * A buffer into which command outputs can be written is declared in the
 * main command interpreter, rather than in the command console implementation,
 * to allow application that provide access to the command console via multiple
 * interfaces to share a buffer, and therefore save RAM.  Note, however, that
 * the command interpreter itself is not re-entrant, so only one command
 * console interface can be used at any one time.  For that reason, no attempt
 * is made to provide any mutual exclusion mechanism on the output buffer.
 *
 * FreeRTOS_CLIGetOutputBuffer() returns the address of the output buffer.
 */
char *FreeRTOS_CLIGetOutputBuffer( void );

/*
 * Return a pointer to the xParameterNumber'th word in pcCommandString.
 */
const char *FreeRTOS_CLIGetParameter( const char *pcCommandString, UBaseType_t uxWantedParameter, BaseType_t *pxParameterStringLength );

#endif /* COMMAND_INTERPRETER_H */













