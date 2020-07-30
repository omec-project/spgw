/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
#include "pfcp_cp_interface.h"

int validate_pfcp_message_content(msg_info_t *msg)
{
    switch(msg->msg_type)
    {
    	case PFCP_ASSOCIATION_SETUP_RESPONSE:
        {
            break;
        }
        case PFCP_ASSOCIATION_UPDATE_RESPONSE:
        case PFCP_ASSOCIATION_RELEASE_RESPONSE:
        case PFCP_PFD_MANAGEMENT_RESPONSE:
        {
            break;
        }
        default:
            break;
    }
    return 0;
}


