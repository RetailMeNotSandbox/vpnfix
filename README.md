fixvpn
======

When using the Cisco AnyConnect client, some network interfaces, including those to virtual machines, will be denied.  This small application fixes the ipfw firewall rules to permit routing to these local interfaces.

Requirements
============

- c compiler (clang, gcc, etc.) for building
- root permissions for running

Usage
=====

To build and install:

    $ make install

After connecting to the VPN with Cisco AnyConnect, run:

    $ sudo fixvpn

Elevated user privileges are required because of querying and modifying the ipfw firewall rules.

Description
===========

The Cisco AnyConnect client creates a network interface that serves as a tunnel for the VPN connection (`utun0`), and adds the following ipfw firewall routing rules:

* Traffic destined to the loopback interface (`lo0`) will be allowed and routed through `lo0`.
* If client is configured for local (LAN) access and the Cisco admin has enabled split tunneling for the VPN, then traffic destined to local (LAN) address are allowed and routed through first enumerated ethernet interface (e.g. `en0`).  This rule allows for routing of unknown destinations, such as when one tries to access `www.google.com`, as routing will attempt to go through the first ethernet interface's default gateway.
* Traffic destined to secured-routes, which are specified by the Cisco admin, are routed through the `utun0` interface.
* All other traffic is denied.  This includes traffic to other interfaces, such as additional NICs or virtual interfaces (`vmnet#`).

As the ipfw firewall is handling routing, simply disabling it is not an option.  However, deleting the deny rule will allow normal routing to occur across additional interfaces, while still routing secured routes through the `utun0` tunnel.

The `ipfw` utility can be used to modify the kernel ipfw firewall.  This utility was deprecated in OS X 10.7 and finally removed in Yosemite in favor of using `pfctl`.  However, Yosemite's kernel supports both ipfw and pf firewalls.  The Cisco AnyConnect client uses the ipfw firewall, and the deny rules can be removed by either:

* installing the `ipfw` application from an older OS X version and use it to remove the deny rules:

        $ sudo ipfw delete $(sudo ipfw -a list | grep deny | cut -f1 -d' ')

* installing `fixvpn`:

        $ make install
        $ sudo fixvpn

For routing to virtual machines, one alternative with some limitations is to use the hypervisor's routing capabilities and enable port-forwarding for NAT adapters so that traffic destined to the host `lo0` interface will get forwarded through the `vnet` interface.  This does not require changing the ipfw firewall rules, but it does have some limitations, such as requiring forwarding rules for each port and not being able to forward from a port below `1024` without running the hypervisor as root.

License
=======

MIT Copyright (c) 2015, 2016 RetailMeNot
