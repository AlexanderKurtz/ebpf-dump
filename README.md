# ebpf-dump - Determine the data available to an eBPF filter for various SOCK_* values

This project tries to determine the data available to an eBPF filter for various
SOCK_* values. For this, we have written three small server programs
(```server-raw.c```, ```server-tcp.c```, and ```server-udp.c```) which open an
```AF_INET``` ```SOCK_RAW```, ```SOCK_STREAM```, and ```SOCK_DGRAM``` socket
respectively and attach an eBPF filter to it. Both the server binary and the
eBPF filter then simply dump the the data they receive in each packet as
hexadecimal values.

## Test setup

The tests were conducted on a machine completely disconnected from any network
and over the loopback interface in order to avoid capturing unrelated data.
Since the eBPF filter program dumps its data to the kernel debug pipe
(```/sys/kernel/debug/tracing/trace_pipe```) (which only priviledged programs
can use) and since we use a ```SOCK_RAW``` socket (which is also only available
to priviledged programs), all tests were run as ```root```.

## Test method

The tests were split up in four steps:

 1. Start listening on the kernel debug pipe (i.e. run ```sudo cat
    /sys/kernel/debug/tracing/trace_pipe```).
 2. Start the server process (i.e. run ```sudo ./server-{raw,tcp,udp}```)
 3. Generate some suitable network traffic (i.e. run ```echo 0123456789 | ncat
    127.0.0.1 16962``` for TCP or ```echo 0123456789 | ncat -u 127.0.0.1
    16962``` for UDP).
 4. Save the output of the kernel debug pipe to
    ```results/{raw,tcp,udp}-{kernelspace,userspace}```.

## Results

The raw results are available in the ```results``` directory. For interpreting
them, it is useful to note that the port number used in all tests (```16962```)
has a hex encoding of ```0x4242```) which means that two consectutive 0x42 bytes
are probably the destination port in the TCP or UDP header. The following
observations can be made by comparing the results from the kernel- and userspace
for each test.

 1. For ```SOCK_RAW``` sockets, the data visible to the kernelspace and
    userspace is completely identical: We see two packets, each starting with an
    IPv4 header and then followed by a TCP header. The IP address (127.0.0.1 =
    0x7f000001) and port (0x4242) used are easily recognized and a second look
    reveals that we are dealing with a a TCP-SYN followed by a TCP-RST packet.
    This matches our expectations since a "raw socket receives or sends the raw
    datagram not including link level headers" (see
    [raw(7)](https://manpages.debian.org/raw.7)) and we didn't install a service
    which listen(2)-ed on the TCP port in question. The only thing worth noting
    is probably that we see both ingress and egress traffic.
 2. For ```SOCK_STREAM``` the data seen by the kernelspace and userspace differs
    noticably. While userspace only sees one packet containing the TCP payload
    (i.e. the ASCII string ```"0123456789\n"```, the kernelspace sees three
    packets, each one starting with a TCP header. Closer inspection of the byte
    at offset 13 reveals that we are seeing a TCP-SYN, then a TCP-ACK+PSH, and
    finally a TCP-ACK+FIN segment. So, we are only seeing the ingress traffic
    and only the TCP segment and not the whole IP packet or ethernet frame.
    While we expected to only see ingress traffic
    ([socket(7)](https://manpages.debian.org/socket.7) says that eBPF filters
    are used as a "filter of incoming packets"), we are surprised by the fact
    that the kernelspace did not get access to the IP or Ethernet headers.
 3. For ```SOCK_DGRAM``` the results are similar to TCP, while the userspace
    only sees the payload, the kernelspace sees the whole UDP packet.

## Conclusion

It seems that the usefulnes of eBPF socket filters is severly limited if they
aren't attached to ```SOCK_RAW``` sockets, since they then only get access to
incoming layer 4 packets and therefore apparantly cannot filter based on
information located in lower level headers or egress traffic. This probably
explains why the socket filtering examples both of the ```bcc``` project (a
library trying to make eBPF usage in general accessible to application
developers) and those of the Linux kernel all use raw sockets instead of TCP or
UDP sockets. To verify this, have a look at the
```examples/networking/{dns_matching,http_filters}``` directories of the [bcc
project](https://github.com/iovisor/bcc/) and the
```samples/bpf/sockex*_user.c``` files of the [Linux
kernel](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git).
