#ifndef COMMON_LIB_H_
#define COMMON_LIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#endif

// parse version, type, and length
void parse_vtl(uint32_t vtl);

void parse_sbcp_attribute(sbcp_attribute_t sbcp_attribute);

void parse_msg_join(sbcp_msg_t msg_join);
