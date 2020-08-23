# Usage Reporting Rules (URR) - WIP
# Supported Measurement Method
- **Volume** based Usage reporting is under developement
- **traffic** duration measurement is not yet supported
- **event threshold** measurement is not yet supported

#Supported Reporting Triggers
- TODO - Add more content here

# Message support
-  URR is configured in the user plane through  PFCP sesssion establishment request and PFCP session modification request message.
-  Usage reports can be fetched from user plane in the **PFCP Session Delete Response** Or user plane can asynchronously report the usage  in the **PFCP session report message** 

# Information Elements in URR
- **Measurement Method IE** - Configure measurement method
- **Reporting Trigger IE** - reporting triggers helps to inform UP when to trigger report
- **Volume Threshold IE**  - to request UP function to generate a usage report when the measured traffic reaches the threshold
- **Volume Quota** - to request the UP function to stop forwarding packets and also to generate usage report when no volume threshold is cconfigured and volume Quota is reached
- **Dropped DL Traffic threshold IE** - to generate usage report when downlink traffic that is being dropped reaches the threshold
- **Measurement information** - "Measurement before QoS enforcement" set then measure usage before QoS
- **Quote Holding Time** - to request UP function to send usage report and to also stop forwarding packets when no packets have been received for the duration indicated in this param.
- **monitoring Timer IE** and **additional monitoring IE** - to request UP function to measure the network resource usage before and after monitoring time in separate counts and to reapply volume threshold at the monitoring time. 
- **subsequent volume threshold IE** 

# Session Report IEs
- UR-SEQN (Usage Report Sequene number) 
- Usage Report Trigger IE 
# Volume Based Measurement Method

# important Notes
- URR should always be complete list. 
- CP function may request any time the UP function to activate or deactivate network usage measurement using **inactive measurement flag** in measurement information IE of URR
 - FAR ID in the Quota Action IE - CP provision this in UP. 
 - Usage report can come in PFCP session Mod Response
