License & Copyright
----

#SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>
#SPDX-License-Identifier: Apache-2.0

# SPGW Supports following
## Control Plane initiated PFCP association setup 
- Buffer CSReq messages while control plane is waiting for response from the user plane.
- if Association setup fails then reject all buffered CSReq message
- if Association setup successful then process buffered CSReq messages and initiate PFCP session establishment request

## Userplane initiated PFCP association setup
- Userplane can pro-actively connect to control plane and exchange IP resources information.
- Control plane can use the readily available user plane connection
##PFCP Association Update
- TODO
##PFCP Association Release
- TODO
