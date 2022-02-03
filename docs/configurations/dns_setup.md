# License & Copyright
#SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>

#Copyright (c) 2019 Sprint

#SPDX-License-Identifier: Apache-2.0

#SPDX-License-Identifier: Apache-2.0

# DNS Server Setup for UPF selection

## Basic DNS Server Installation
1. Install the required DNS packages
sudo apt-get install bind9 dnsutils bind9-doc
2. Set bind to IPV4 mode by update the options parameter in /etc/default/bind9 as follows:
OPTIONS="-4 -u bind"
3. Modify /etc/bind/named.conf.options to define the DNS servers to forward unresolved DNS request to.  For example to forward unresolved DNS requests to Google, /etc/bind/named.conf.options should look as follows:
options {
        directory "/var/cache/bind";

        // If there is a firewall between you and nameservers you want
        // to talk to, you may need to fix the firewall to allow multiple
        // ports to talk.  See http://www.kb.cert.org/vuls/id/800113

        // If your ISP provided one or more IP addresses for stable
        // nameservers, you probably want to use them as forwarders.
        // Uncomment the following block, and insert the addresses replacing
        // the all-0's placeholder.

        forwarders {
             8.8.8.8;
        };

        //========================================================================
        // If BIND logs error messages about the root key being expired,
        // you will need to update your keys.  See https://www.isc.org/bind-keys
        //========================================================================
        dnssec-validation auto;

        auth-nxdomain no;    # conform to RFC1035
        listen-on-v6 { any; };
};

4. Configure Local File.  Modify /etc/bind/named.conf.local as follows:
Add the forward zone with the following lines (substitute the zone name with your own):
zone "test3gpp.net" {
 type master;
 file "/etc/bind/db.test3gpp.net";
};
Add the reverse zone by adding the following lines (note that the reverse zone name starts with 93.212.10 which is the opposite of 10.212.93.10).
zone "93.212.10in-addr.arpa" {
 type master;
 notify no;
 file "/etc/bind/db.10";
};

5. Create the forward zone file using the file name referenced in /etc/bind/named.conf.local.  In this example the file is named /etc/bind/db.test3gpp.net and the DNS server (ns1),;
; BIND data file for local loopback interface
;
$TTL    604800
@       IN      SOA     ns1.test3gpp.net. root.ns1.test3gpp.net. (
                              2         ; Serial
                         604800         ; Refresh
                          86400         ; Retry
                        2419200         ; Expire
                         604800 )       ; Negative Cache TTL
;
@       IN      NS      ns1.test3gpp.net.
@       IN      A       127.0.0.1
@       IN      AAAA    ::1


6. Create the reverse zone file using the file name referenced in /etc/bind/named.conf.local.  In this example the file is named /etc/bind/db.10
;
; BIND reverse data file for local loopback interface
;
$TTL    604800
@       IN      SOA     ns1.test3gpp.net. root.ts1.test3gpp.net. (
                              1         ; Serial
                         604800         ; Refresh
                          86400         ; Retry
                        2419200         ; Expire
                         604800 )       ; Negative Cache TTL
;
@       IN      NS      ns1.test3gpp.net.

7. Start the DNS server.
sudo service bind9 start
8. Verify that the DNS service started successfully by opening /var/log/syslog and look for the following entry toward the end of the file:
Aug 10 13:20:29 ns1 named[29932]: all zones loaded
If this message cannot be located, locate the error message associated with the “named” service and make the necessary corrections.

## Update /etc/hostname and /etc/hosts
1. Modify /etc/hostname to reflect the unqualified host name of each server.  In this example, the DNS server hostname should be:
ns1
 2. Ensure that /etc/hosts has entries for 127.0.0.1, 127.0.1.1 and the IP address of the host with both the fully qualified hostname and the unqualified hostname.
127.0.0.1	localhost
127.0.1.1	ns1.test3gpp.net	ns1
10.212.93.10	ns1.test3gpp.net	ns1





## Sample NAPTR Record Configuration:
Edit /etc/bind/db.test3gpp.net file as per following instructions. Restart bind9 service to apply new configuration.
#sudo service bind9 restart

### For SAEGW-U selection:
Following is the example of DNS configuration records for selecting SAEGW-U. Highlighted configuration is for selecting 
a) 192.168.0.46 and 192.168.0.96 as SGW-U for enodeb id = 1, mnc = 123, mcc = 310 and network capability = lbo. 
b) 192.168.0.46, 192.168.0.96 and 192.168.0.56 as PGW-U for apn = apn1, mnc = 123, mcc = 310 and network capability = lbo. 
c) After collocation determination between SGW-U list and PGW-U list, 192.168.0.46 will be selected as SAEGW-U. 

topon.eth1.enb1.edgea.nodes IN A 192.0.2.151
                       IN A 192.0.2.152
                       IN AAAA 2001:db8:0:0e:0:0:0:0
                       IN AAAA 2001:db8:0:0f:0:0:0:0

topoff.eth5.gw01.nodes IN A 192.0.2.153
                       IN A 192.0.2.154
                       IN AAAA 2001:db8:0:1e:0:0:0:0
                       IN AAAA 2001:db8:0:1f:0:0:0:0

topon.eth4.gw04.edgea.nodes  IN A 192.168.0.96
                       IN AAAA 2001:db8:0:42:0:0:0:0

topon.eth5.gw02.nodes  IN A 192.168.0.56
                       IN AAAA 2001:db8:0:44:0:0:0:0		   

topon.eth4.gw03.edgea.nodes  IN A 192.168.0.46
                       IN AAAA 2001:db8:0:24:0:0:0:0

enb1.enb.epc.mnc123.mcc310 (
;  IN NAPTR order pref. flag service                           regexp replacement
   IN NAPTR 800   100   "s" "x-3gpp-node"                          "" topon.eth1.enb1.edgea.nodes )
   IN NAPTR 800   100   "a" "x-3gpp-upf:x-sxa+nc-lbo"              "" topon.eth4.gw04.edgea.nodes
   IN NAPTR 700   100   "a" "x-3gpp-upf:x-sxa+nc-lbo"              "" topon.eth4.gw03.edgea.nodes
					   
apn1.apn.epc.mnc123.mcc310  (
;  IN NAPTR order pref. flag service                           regexp replacement
   IN NAPTR 800   100   "a" "x-3gpp-upf:x-sxb"                     "" topoff.eth5.gw01.nodes )
   IN NAPTR 800   200   "a" "x-3gpp-upf:x-sxb+nc-lbo"              "" topon.eth5.gw02.nodes
   IN NAPTR 800   100   "a" "x-3gpp-upf:x-sxb+nc-lbo"              "" topon.eth4.gw03.edgea.nodes
   IN NAPTR 800   100   "a" "x-3gpp-upf:x-sxb+nc-lbo"              "" topon.eth4.gw04.edgea.nodes

### For SGW-U selection:
Following is the example of DNS configuration records for selecting SGW-U. Highlighted configuration is for selecting 
192.168.0.16 as SGW-U for enodeb id = 103250, mnc = 01, mcc = 803 and network capability = lbo. 
topon.eth9.gw03.edgea.nodes will be sent as SGW-U node name IE in Create Session Request to PGW-C.


topon.eth4.gw03.edgea.nodes  IN A 10.0.4.30
                        IN AAAA 2001:db8:0:22:0:0:0:0

topon.eth9.gw03.edgea.nodes  IN A 192.168.0.16
                        IN AAAA 2001:db8:0:22:0:0:0:0
					   
enb103250.enb.epc.mnc01.mcc803 (
;  IN NAPTR order pref. flag service                           regexp replacement
   IN NAPTR 800   100   "s" "x-3gpp-node"                          "" topon.eth1.enb1.edgea.nodes )
   IN NAPTR 720   100   "a" "x-3gpp-upf:x-sxa+nc-lbo"              "" topon.eth9.gw03.edgea.nodes
   IN NAPTR 800   100   "a" "x-3gpp-upf:x-sxa+nc-lbo"              "" topon.eth4.gw03.edgea.nodes


### For PGW-U selection:
Following is the example of DNS configuration records for selecting PGW-U. Highlighted configuration is for selecting 
192.168.0.31 and 10.0.4.30 as PGW-U for apn = apn1, mnc = 01, mcc = 803 and network capability = lbo. 
After collocation determination between SGW-U node name IE in Create Session Request and PGW-U list, 192.168.0.31 will be selected as PGW-U. 

topon.eth4.gw03.edgea.nodes  IN A 10.0.4.30
                        IN AAAA 2001:db8:0:22:0:0:0:0

topon.eth10.gw03.edgea.nodes  IN A 192.168.0.31
                       ;IN A 192.0.2.136
                       IN AAAA 2001:db8:0:22:0:0:0:0

apn1.apn.epc.mnc01.mcc803  (
;  IN NAPTR order pref. flag service                           regexp replacement
   IN NAPTR 800   100   "a" "x-3gpp-upf:x-sxb"                     "" topoff.eth5.gw01.nodes )
   IN NAPTR 700   100   "a" "x-3gpp-upf:x-sxb+nc-lbo"              "" topon.eth10.gw03.edgea.nodes
   IN NAPTR 800   100   "a" "x-3gpp-upf:x-sxb+nc-lbo"              "" topon.eth4.gw02.edgea.nodes
