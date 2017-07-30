#include <bcc/helpers.h>

#define MAX_PACKET_SIZE 128

int filter(struct __sk_buff *skb) {
	/* Dump the packet to the kernel debug pipe */
	bpf_trace_printk ("Kernelspace: Received %d bytes, dumping\n", skb->len);
	# pragma unroll
	for (int i = 0; i < MAX_PACKET_SIZE; i ++) {
		if (i < skb->len) {
			int value = load_byte (skb, i);
			bpf_trace_printk ("\tbyte[%d]=0x%x\n", i, value);
		} else {
			break;
		}
	}
	bpf_trace_printk ("\n");

	/* Keep the packet without truncating it */
	return skb->len;
}
