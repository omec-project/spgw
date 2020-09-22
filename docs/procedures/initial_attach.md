# Initial Attach Procedure

# Testing

    1. Attach Success
    2. CSReq validation failed (e.g. bad apn, no ip address,.....)
    3. PFCP setup timeout....
    4. PFCP setup reject
    5. PFCP session setup reject
    6. PFCP session setup timeout
    7. Context Replacement while first CSReq was complete
    7a. Context Replacement while first CSReq was in progress
    8. Retransmitted CSReq while first CSReq is still in progress. Just drop CSReq and log a message
    9. Attach request received, but subscriber classification config is not yet available.
      So leads to reject CSReq
    10. Second UE attach while PFCP session in progress 
    11. Second UE attach while PFCP session is already established 
   

# Pending tasks
    1. Graceful cleanup when context replacement. 

# Special cases which needs more investigation
    1. setup response has different nodeid or FQDN. 
