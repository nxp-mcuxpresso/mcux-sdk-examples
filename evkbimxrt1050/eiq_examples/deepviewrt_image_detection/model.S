	.section .rodata

	.global model_rtm_start
	.global model_rtm_end

	.global sample_img_start
	.global sample_img_end
	
	.align 8
model_rtm_start:
	.incbin "models/mobilenet_ssd_v2_lite_quant_anchors.rtm"
model_rtm_end:
sample_img_start:
	.incbin "data/horse.jpg"
sample_img_end:
