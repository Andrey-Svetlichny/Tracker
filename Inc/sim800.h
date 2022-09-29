#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SIM800_MAX_COMMAND_LEN 255  ///< Maximum command length
#define SIM800_MAX_RESPONSE_LEN 255 ///< Maximum response length

#define SIM800_RESULT_NO_RESULT_YET 0           ///< command executing
#define SIM800_RESULT_SUCCESS 1                 ///< command successful
#define SIM800_RESULT_ERROR 2                   ///< SIM800 reported error
#define SIM800_RESULT_RESPONSE_NOT_RECOGNIZED 3 ///< SIM800 response differ from OK, ERROR and expected result
#define SIM800_RESULT_TIMEOUT 4                 ///< timeout
#define SIM800_RESULT_COMMAND_TOO_LONG 5        ///< command exceed SIM800_MAX_COMMAND_LEN
#define SIM800_RESULT_ANOTHER_EXECUTING 6       ///< another command execution not finished

typedef struct sim800_t sim800_t;
typedef struct sim800_t
{
    char command[SIM800_MAX_COMMAND_LEN];   ///< command
    bool executing;                         ///< command running
    char response[SIM800_MAX_RESPONSE_LEN]; ///< response
    uint8_t response_len;                   ///< length of response, 0 means ready to receive
    uint8_t result;                         ///< command result - SIM800_RESULT_XXX
    char *resultData;                       ///< response result data
    void (*onError)(sim800_t *self);
    void (*transmit)(char *cmd);
} sim800_t;

bool sim800_connect(sim800_t *p);
bool sim800_send(sim800_t *p, char *msg);
void sim800_disconnect(sim800_t *p);
