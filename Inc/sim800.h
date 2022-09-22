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
    uint8_t command[SIM800_MAX_COMMAND_LEN];   ///< command
    bool executing;                            ///< command running
    uint8_t response[SIM800_MAX_RESPONSE_LEN]; ///< response
    uint8_t response_len;                      ///< length of response
    uint8_t result;                            ///< command result - SIM800_RESULT_XXX
    uint8_t *resultData;                       ///< response result data
    void (*onError)(sim800_t *self);
    void (*transmit)(char *cmd);
    void (*parse)(sim800_t *self); ///< parse response, set executing, result, result_data
} sim800_t;

void sim800_receiveChar(uint8_t c, sim800_t *p);
bool sim800_connect(sim800_t *p);
bool sim800_send(sim800_t *p, char *msg);
void sim800_disconnect(sim800_t *p);
