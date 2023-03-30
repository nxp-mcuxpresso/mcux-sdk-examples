	.section .rodata

	.global deepview_rtm_start
	.global deepview_rtm_end

	.align 8
deepview_rtm_start:
	.incbin "models/mobilenet_v1_0.25_160.rtm"
deepview_rtm_end:

