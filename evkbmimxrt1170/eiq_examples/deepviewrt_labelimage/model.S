	.section .rodata

	.global model_rtm_start
	.global model_rtm_end

	.global sample_img_start
	.global sample_img_end
	
	.align 8
model_rtm_start:
	.incbin "models/mobilenet_v1_0.25_224_quant.rtm"
model_rtm_end:
sample_img_start:
	.incbin "data/sample.jpg"
sample_img_end:
