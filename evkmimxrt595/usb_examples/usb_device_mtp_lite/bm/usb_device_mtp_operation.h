/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_MTP_OPERATION_H__
#define __USB_DEVICE_MTP_OPERATION_H__

/*!
 * @addtogroup mtp_spec
 * @{
 */

/*!< @brief MTP data type code*/
#define MTP_TYPE_UNDEFINED 0x0000U /*!< Undefined                          */
#define MTP_TYPE_INT8      0x0001U /*!< Signed 8-bit integer               */
#define MTP_TYPE_UINT8     0x0002U /*!< Unsigned 8-bit integer             */
#define MTP_TYPE_INT16     0x0003U /*!< Signed 16-bit integer              */
#define MTP_TYPE_UINT16    0x0004U /*!< Unsigned 16-bit integer            */
#define MTP_TYPE_INT32     0x0005U /*!< Signed 32-bit integer              */
#define MTP_TYPE_UINT32    0x0006U /*!< Unsigned 32-bit integer            */
#define MTP_TYPE_INT64     0x0007U /*!< Signed 64-bit integer              */
#define MTP_TYPE_UINT64    0x0008U /*!< Unsigned 64-bit integer            */
#define MTP_TYPE_INT128    0x0009U /*!< Signed 128-bit integer             */
#define MTP_TYPE_UINT128   0x000AU /*!< Unsigned 128-bit integer           */
#define MTP_TYPE_AINT8     0x4001U /*!< Array of signed 8-bit integers     */
#define MTP_TYPE_AUINT8    0x4002U /*!< Array of unsigned 8-bit integers   */
#define MTP_TYPE_AINT16    0x4003U /*!< Array of signed 16-bit integers    */
#define MTP_TYPE_AUINT16   0x4004U /*!< Array of unsigned 16-bit integers  */
#define MTP_TYPE_AINT32    0x4005U /*!< Array of signed 32-bit integers    */
#define MTP_TYPE_AUINT32   0x4006U /*!< Array of unsigned 32-bit integers  */
#define MTP_TYPE_AINT64    0x4007U /*!< Array of signed 64-bit integers    */
#define MTP_TYPE_AUINT64   0x4008U /*!< Array of unsigned 64-bit integers  */
#define MTP_TYPE_AINT128   0x4009U /*!< Array of signed 128-bit integers   */
#define MTP_TYPE_AUINT128  0x400AU /*!< Array of unsigned 128-bit integers */
#define MTP_TYPE_STR       0xFFFFU /*!< Variable-length Unicode string     */

/*!< @brief MTP functional mode */
#define MTP_FUNCTIONAL_MODE_STANDARD_MODE           0x0000U
#define MTP_FUNCTIONAL_MODE_SLEEP_MODE              0x0001U
#define MTP_FUNCTIONAL_MODE_NON_RESPONSIVE_PLAYBACK 0xC001U
#define MTP_FUNCTIONAL_MODE_RESPONSIVE_PLAYBACK     0xC002U

/*! @brief MTP format code */
#define MTP_FORMAT_UNDEFINED                         0x3000U
#define MTP_FORMAT_ASSOCIATION                       0x3001U
#define MTP_FORMAT_SCRIPT                            0x3002U
#define MTP_FORMAT_EXECUTABLE                        0x3003U
#define MTP_FORMAT_TEXT                              0x3004U
#define MTP_FORMAT_HTML                              0x3005U
#define MTP_FORMAT_DPOF                              0x3006U
#define MTP_FORMAT_AIFF                              0x3007U
#define MTP_FORMAT_WAV                               0x3008U
#define MTP_FORMAT_MP3                               0x3009U
#define MTP_FORMAT_AVI                               0x300AU
#define MTP_FORMAT_MPEG                              0x300BU
#define MTP_FORMAT_ASF                               0x300CU
#define MTP_FORMAT_UNDEFINED_IMAGE                   0x3800U
#define MTP_FORMAT_EXIF_JPEG                         0x3801U
#define MTP_FORMAT_TIFF_EP                           0x3802U
#define MTP_FORMAT_FLASHPIX                          0x3803U
#define MTP_FORMAT_BMP                               0x3804U
#define MTP_FORMAT_CIFF                              0x3805U
#define MTP_FORMAT_UNDEFINED_1                       0x3806U
#define MTP_FORMAT_GIF                               0x3807U
#define MTP_FORMAT_JFIF                              0x3808U
#define MTP_FORMAT_CD                                0x3809U
#define MTP_FORMAT_PICT                              0x380AU
#define MTP_FORMAT_PNG                               0x380BU
#define MTP_FORMAT_UNDEFINED_2                       0x380CU
#define MTP_FORMAT_TIFF                              0x380DU
#define MTP_FORMAT_TIFF_IT                           0x380EU
#define MTP_FORMAT_JP2                               0x380FU
#define MTP_FORMAT_JPX                               0x3810U
#define MTP_FORMAT_UNDEFINED_FIRMWARE                0xB802U
#define MTP_FORMAT_WBMP                              0xB803U
#define MTP_FORMAT_JPEG_XR                           0xB804U
#define MTP_FORMAT_WINDOWS_IMAGE_FORMAT              0xB881U
#define MTP_FORMAT_UNDEFINED_AUDIO                   0xB900U
#define MTP_FORMAT_WMA                               0xB901U
#define MTP_FORMAT_OGG                               0xB902U
#define MTP_FORMAT_AAC                               0xB903U
#define MTP_FORMAT_AUDIBLE                           0xB904U
#define MTP_FORMAT_FLAC                              0xB906U
#define MTP_FORMAT_QCELP                             0xB907U
#define MTP_FORMAT_AMR                               0xB908U
#define MTP_FORMAT_UNDEFINED_VIDEO                   0xB980U
#define MTP_FORMAT_WMV                               0xB981U
#define MTP_FORMAT_MP4_CONTAINER                     0xB982U
#define MTP_FORMAT_MP2                               0xB983U
#define MTP_FORMAT_3GP_CONTAINER                     0xB984U
#define MTP_FORMAT_3GP2                              0xB985U
#define MTP_FORMAT_AVCHD                             0xB986U
#define MTP_FORMAT_ATSC_TS                           0xB987U
#define MTP_FORMAT_DVB_TS                            0xB988U
#define MTP_FORMAT_UNDEFINED_COLLECTION              0xBA00U
#define MTP_FORMAT_ABSTRACT_MULTIMEDIA_ALBUM         0xBA01U
#define MTP_FORMAT_ABSTRACT_IMAGE_ALBUM              0xBA02U
#define MTP_FORMAT_ABSTRACT_AUDIO_ALBUM              0xBA03U
#define MTP_FORMAT_ABSTRACT_VIDEO_ALBUM              0xBA04U
#define MTP_FORMAT_ABSTRACT_AUDIO_VIDEO_PLAYLIST     0xBA05U
#define MTP_FORMAT_ABSTRACT_CONTACT_GROUP            0xBA06U
#define MTP_FORMAT_ABSTRACT_MESSAGE_FOLDER           0xBA07U
#define MTP_FORMAT_ABSTRACT_CHAPTERED_PRODUCTION     0xBA08U
#define MTP_FORMAT_ABSTRACT_AUDIO_PLAYLIST           0xBA09U
#define MTP_FORMAT_ABSTRACT_VIDEO_PLAYLIST           0xBA0AU
#define MTP_FORMAT_ABSTRACT_MEDIACAST                0xBA0BU
#define MTP_FORMAT_WPL_PLAYLIST                      0xBA10U
#define MTP_FORMAT_M3U_PLAYLIST                      0xBA11U
#define MTP_FORMAT_MPL_PLAYLIST                      0xBA12U
#define MTP_FORMAT_ASX_PLAYLIST                      0xBA13U
#define MTP_FORMAT_PLS_PLAYLIST                      0xBA14U
#define MTP_FORMAT_UNDEFINED_DOCUMENT                0xBA80U
#define MTP_FORMAT_ABSTRACT_DOCUMENT                 0xBA81U
#define MTP_FORMAT_XML_DOCUMENT                      0xBA82U
#define MTP_FORMAT_MICROSOFT_WORD_DOCUMENT           0xBA83U
#define MTP_FORMAT_MHT_COMPILED_HTML_DOCUMENT        0xBA84U
#define MTP_FORMAT_MICROSOFT_EXCEL_SPREADSHEET       0xBA85U
#define MTP_FORMAT_MICROSOFT_POWERPOINT_PRESENTATION 0xBA86U
#define MTP_FORMAT_UNDEFINED_MESSAGE                 0xBB00U
#define MTP_FORMAT_ABSTRACT_MESSSAGE                 0xBB01U
#define MTP_FORMAT_UNDEFINED_BOOKMARK                0xBB10U
#define MTP_FORMAT_ABSTRACT_BOOKMARK                 0xBB11U
#define MTP_FORMAT_UNDEFINED_APPOINTMENT             0xBB20U
#define MTP_FORMAT_ABSTRACT_APPOINTMENT              0xBB21U
#define MTP_FORMAT_VCALENDAR_1_0                     0xBB22U
#define MTP_FORMAT_UNDEFINED_TASK                    0xBB40U
#define MTP_FORMAT_ABSTRACT_TASK                     0xBB41U
#define MTP_FORMAT_ICALENDAR                         0xBB42U
#define MTP_FORMAT_UNDEFINED_CONTACT                 0xBB80U
#define MTP_FORMAT_ABSTRACT_CONTACT                  0xBB81U
#define MTP_FORMAT_VCARD_2                           0xBB82U
#define MTP_FORMAT_VCARD_3                           0xBB83U

/*! @brief MTP object property code */
#define MTP_OBJECT_PROPERTY_STORAGE_ID                          0xDC01U
#define MTP_OBJECT_PROPERTY_OBJECT_FORMAT                       0xDC02U
#define MTP_OBJECT_PROPERTY_PROTECTION_STATUS                   0xDC03U
#define MTP_OBJECT_PROPERTY_OBJECT_SIZE                         0xDC04U
#define MTP_OBJECT_PROPERTY_ASSOCIATION_TYPE                    0xDC05U
#define MTP_OBJECT_PROPERTY_ASSOCIATION_DESC                    0xDC06U
#define MTP_OBJECT_PROPERTY_OBJECT_FILE_NAME                    0xDC07U
#define MTP_OBJECT_PROPERTY_DATE_CREATED                        0xDC08U
#define MTP_OBJECT_PROPERTY_DATE_MODIFIED                       0xDC09U
#define MTP_OBJECT_PROPERTY_KEYWORDS                            0xDC0AU
#define MTP_OBJECT_PROPERTY_PARENT_OBJECT                       0xDC0BU
#define MTP_OBJECT_PROPERTY_ALLOWED_FOLDER_CONTENTS             0xDC0CU
#define MTP_OBJECT_PROPERTY_HIDDEN                              0xDC0DU
#define MTP_OBJECT_PROPERTY_SYSTEM_OBJECT                       0xDC0EU
#define MTP_OBJECT_PROPERTY_PERSISTENT_UID                      0xDC41U
#define MTP_OBJECT_PROPERTY_SYNC_ID                             0xDC42U
#define MTP_OBJECT_PROPERTY_PROPERTY_BAG                        0xDC43U
#define MTP_OBJECT_PROPERTY_NAME                                0xDC44U
#define MTP_OBJECT_PROPERTY_CREATED_BY                          0xDC45U
#define MTP_OBJECT_PROPERTY_ARTIST                              0xDC46U
#define MTP_OBJECT_PROPERTY_DATE_AUTHORED                       0xDC47U
#define MTP_OBJECT_PROPERTY_DESCRIPTION                         0xDC48U
#define MTP_OBJECT_PROPERTY_URL_REFERENCE                       0xDC49U
#define MTP_OBJECT_PROPERTY_LANGUAGE_LOCALE                     0xDC4AU
#define MTP_OBJECT_PROPERTY_COPYRIGHT_INFORMATION               0xDC4BU
#define MTP_OBJECT_PROPERTY_SOURCE                              0xDC4CU
#define MTP_OBJECT_PROPERTY_ORIGIN_LOCATION                     0xDC4DU
#define MTP_OBJECT_PROPERTY_DATE_ADDED                          0xDC4EU
#define MTP_OBJECT_PROPERTY_NON_CONSUMABLE                      0xDC4FU
#define MTP_OBJECT_PROPERTY_CORRUPT_UNPLAYABLE                  0xDC50U
#define MTP_OBJECT_PROPERTY_PRODUCER_SERIAL_NUMBER              0xDC51U
#define MTP_OBJECT_PROPERTY_REPRESENTATIVE_SAMPLE_FORMAT        0xDC81U
#define MTP_OBJECT_PROPERTY_REPRESENTATIVE_SAMPLE_SIZE          0xDC82U
#define MTP_OBJECT_PROPERTY_REPRESENTATIVE_SAMPLE_HEIGHT        0xDC83U
#define MTP_OBJECT_PROPERTY_REPRESENTATIVE_SAMPLE_WIDTH         0xDC84U
#define MTP_OBJECT_PROPERTY_REPRESENTATIVE_SAMPLE_DURATION      0xDC85U
#define MTP_OBJECT_PROPERTY_REPRESENTATIVE_SAMPLE_DATA          0xDC86U
#define MTP_OBJECT_PROPERTY_WIDTH                               0xDC87U
#define MTP_OBJECT_PROPERTY_HEIGHT                              0xDC88U
#define MTP_OBJECT_PROPERTY_DURATION                            0xDC89U
#define MTP_OBJECT_PROPERTY_RATING                              0xDC8AU
#define MTP_OBJECT_PROPERTY_TRACK                               0xDC8BU
#define MTP_OBJECT_PROPERTY_GENRE                               0xDC8CU
#define MTP_OBJECT_PROPERTY_CREDITS                             0xDC8DU
#define MTP_OBJECT_PROPERTY_LYRICS                              0xDC8EU
#define MTP_OBJECT_PROPERTY_SUBSCRIPTION_CONTENT_ID             0xDC8FU
#define MTP_OBJECT_PROPERTY_PRODUCED_BY                         0xDC90U
#define MTP_OBJECT_PROPERTY_USE_COUNT                           0xDC91U
#define MTP_OBJECT_PROPERTY_SKIP_COUNT                          0xDC92U
#define MTP_OBJECT_PROPERTY_LAST_ACCESSED                       0xDC93U
#define MTP_OBJECT_PROPERTY_PARENTAL_RATING                     0xDC94U
#define MTP_OBJECT_PROPERTY_META_GENRE                          0xDC95U
#define MTP_OBJECT_PROPERTY_COMPOSER                            0xDC96U
#define MTP_OBJECT_PROPERTY_EFFECTIVE_RATING                    0xDC97U
#define MTP_OBJECT_PROPERTY_SUBTITLE                            0xDC98U
#define MTP_OBJECT_PROPERTY_ORIGINAL_RELEASE_DATE               0xDC99U
#define MTP_OBJECT_PROPERTY_ALBUM_NAME                          0xDC9AU
#define MTP_OBJECT_PROPERTY_ALBUM_ARTIST                        0xDC9BU
#define MTP_OBJECT_PROPERTY_MOOD                                0xDC9CU
#define MTP_OBJECT_PROPERTY_DRM_STATUS                          0xDC9DU
#define MTP_OBJECT_PROPERTY_SUB_DESCRIPTION                     0xDC9EU
#define MTP_OBJECT_PROPERTY_IS_CROPPED                          0xDCD1U
#define MTP_OBJECT_PROPERTY_IS_COLOUR_CORRECTED                 0xDCD2U
#define MTP_OBJECT_PROPERTY_IMAGE_BIT_DEPTH                     0xDCD3U
#define MTP_OBJECT_PROPERTY_FNUMBER                             0xDCD4U
#define MTP_OBJECT_PROPERTY_EXPOSURE_TIME                       0xDCD5U
#define MTP_OBJECT_PROPERTY_EXPOSURE_INDEX                      0xDCD6U
#define MTP_OBJECT_PROPERTY_TOTAL_BITRATE                       0xDE91U
#define MTP_OBJECT_PROPERTY_BITRATE_TYPE                        0xDE92U
#define MTP_OBJECT_PROPERTY_SAMPLE_RATE                         0xDE93U
#define MTP_OBJECT_PROPERTY_NUMBER_OF_CHANNELS                  0xDE94U
#define MTP_OBJECT_PROPERTY_AUDIO_BIT_DEPTH                     0xDE95U
#define MTP_OBJECT_PROPERTY_SCAN_TYPE                           0xDE97U
#define MTP_OBJECT_PROPERTY_AUDIO_WAVE_CODEC                    0xDE99U
#define MTP_OBJECT_PROPERTY_AUDIO_BITRATE                       0xDE9AU
#define MTP_OBJECT_PROPERTY_VIDEO_FOURCC_CODEC                  0xDE9BU
#define MTP_OBJECT_PROPERTY_VIDEO_BITRATE                       0xDE9CU
#define MTP_OBJECT_PROPERTY_FRAMES_PER_THOUSAND_SECONDS         0xDE9DU
#define MTP_OBJECT_PROPERTY_KEYFRAME_DISTANCE                   0xDE9EU
#define MTP_OBJECT_PROPERTY_BUFFER_SIZE                         0xDE9FU
#define MTP_OBJECT_PROPERTY_ENCODING_QUALITY                    0xDEA0U
#define MTP_OBJECT_PROPERTY_ENCODING_PROFILE                    0xDEA1U
#define MTP_OBJECT_PROPERTY_DISPLAY_NAME                        0xDCE0U
#define MTP_OBJECT_PROPERTY_BODY_TEXT                           0xDCE1U
#define MTP_OBJECT_PROPERTY_SUBJECT                             0xDCE2U
#define MTP_OBJECT_PROPERTY_PRIORITY                            0xDCE3U
#define MTP_OBJECT_PROPERTY_GIVEN_NAME                          0xDD00U
#define MTP_OBJECT_PROPERTY_MIDDLE_NAMES                        0xDD01U
#define MTP_OBJECT_PROPERTY_FAMILY_NAME                         0xDD02U
#define MTP_OBJECT_PROPERTY_PREFIX                              0xDD03U
#define MTP_OBJECT_PROPERTY_SUFFIX                              0xDD04U
#define MTP_OBJECT_PROPERTY_PHONETIC_GIVEN_NAME                 0xDD05U
#define MTP_OBJECT_PROPERTY_PHONETIC_FAMILY_NAME                0xDD06U
#define MTP_OBJECT_PROPERTY_EMAIL_PRIMARY                       0xDD07U
#define MTP_OBJECT_PROPERTY_EMAIL_PERSONAL_1                    0xDD08U
#define MTP_OBJECT_PROPERTY_EMAIL_PERSONAL_2                    0xDD09U
#define MTP_OBJECT_PROPERTY_EMAIL_BUSINESS_1                    0xDD0AU
#define MTP_OBJECT_PROPERTY_EMAIL_BUSINESS_2                    0xDD0BU
#define MTP_OBJECT_PROPERTY_EMAIL_OTHERS                        0xDD0CU
#define MTP_OBJECT_PROPERTY_PHONE_NUMBER_PRIMARY                0xDD0DU
#define MTP_OBJECT_PROPERTY_PHONE_NUMBER_PERSONAL               0xDD0EU
#define MTP_OBJECT_PROPERTY_PHONE_NUMBER_PERSONAL_2             0xDD0FU
#define MTP_OBJECT_PROPERTY_PHONE_NUMBER_BUSINESS               0xDD10U
#define MTP_OBJECT_PROPERTY_PHONE_NUMBER_BUSINESS_2             0xDD11U
#define MTP_OBJECT_PROPERTY_PHONE_NUMBER_MOBILE                 0xDD12U
#define MTP_OBJECT_PROPERTY_PHONE_NUMBER_MOBILE_2               0xDD13U
#define MTP_OBJECT_PROPERTY_FAX_NUMBER_PRIMARY                  0xDD14U
#define MTP_OBJECT_PROPERTY_FAX_NUMBER_PERSONAL                 0xDD15U
#define MTP_OBJECT_PROPERTY_FAX_NUMBER_BUSINESS                 0xDD16U
#define MTP_OBJECT_PROPERTY_PAGER_NUMBER                        0xDD17U
#define MTP_OBJECT_PROPERTY_PHONE_NUMBER_OTHERS                 0xDD18U
#define MTP_OBJECT_PROPERTY_PRIMARY_WEB_ADDRESS                 0xDD19U
#define MTP_OBJECT_PROPERTY_PERSONAL_WEB_ADDRESS                0xDD1AU
#define MTP_OBJECT_PROPERTY_BUSINESS_WEB_ADDRESS                0xDD1BU
#define MTP_OBJECT_PROPERTY_INSTANT_MESSANGER_ADDRESS           0xDD1CU
#define MTP_OBJECT_PROPERTY_INSTANT_MESSANGER_ADDRESS_2         0xDD1DU
#define MTP_OBJECT_PROPERTY_INSTANT_MESSANGER_ADDRESS_3         0xDD1EU
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_PERSONAL_FULL        0xDD1FU
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_PERSONAL_LINE_1      0xDD20U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_PERSONAL_LINE_2      0xDD21U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_PERSONAL_CITY        0xDD22U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_PERSONAL_REGION      0xDD23U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_PERSONAL_POSTAL_CODE 0xDD24U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_PERSONAL_COUNTRY     0xDD25U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_BUSINESS_FULL        0xDD26U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_BUSINESS_LINE_1      0xDD27U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_BUSINESS_LINE_2      0xDD28U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_BUSINESS_CITY        0xDD29U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_BUSINESS_REGION      0xDD2AU
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_BUSINESS_POSTAL_CODE 0xDD2BU
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_BUSINESS_COUNTRY     0xDD2CU
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_OTHER_FULL           0xDD2DU
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_OTHER_LINE_1         0xDD2EU
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_OTHER_LINE_2         0xDD2FU
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_OTHER_CITY           0xDD30U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_OTHER_REGION         0xDD31U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_OTHER_POSTAL_CODE    0xDD32U
#define MTP_OBJECT_PROPERTY_POSTAL_ADDRESS_OTHER_COUNTRY        0xDD33U
#define MTP_OBJECT_PROPERTY_ORGANIZATION_NAME                   0xDD34U
#define MTP_OBJECT_PROPERTY_PHONETIC_ORGANIZATION_NAME          0xDD35U
#define MTP_OBJECT_PROPERTY_ROLE                                0xDD36U
#define MTP_OBJECT_PROPERTY_BIRTHDATE                           0xDD37U
#define MTP_OBJECT_PROPERTY_MESSAGE_TO                          0xDD40U
#define MTP_OBJECT_PROPERTY_MESSAGE_CC                          0xDD41U
#define MTP_OBJECT_PROPERTY_MESSAGE_BCC                         0xDD42U
#define MTP_OBJECT_PROPERTY_MESSAGE_READ                        0xDD43U
#define MTP_OBJECT_PROPERTY_MESSAGE_RECEIVED_TIME               0xDD44U
#define MTP_OBJECT_PROPERTY_MESSAGE_SENDER                      0xDD45U
#define MTP_OBJECT_PROPERTY_ACTIVITY_BEGIN_TIME                 0xDD50U
#define MTP_OBJECT_PROPERTY_ACTIVITY_END_TIME                   0xDD51U
#define MTP_OBJECT_PROPERTY_ACTIVITY_LOCATION                   0xDD52U
#define MTP_OBJECT_PROPERTY_ACTIVITY_REQUIRED_ATTENDEES         0xDD54U
#define MTP_OBJECT_PROPERTY_ACTIVITY_OPTIONAL_ATTENDEES         0xDD55U
#define MTP_OBJECT_PROPERTY_ACTIVITY_RESOURCES                  0xDD56U
#define MTP_OBJECT_PROPERTY_ACTIVITY_ACCEPTED                   0xDD57U
#define MTP_OBJECT_PROPERTY_ACTIVITY_TENTATIVE                  0xDD58U
#define MTP_OBJECT_PROPERTY_ACTIVITY_DECLINED                   0xDD59U
#define MTP_OBJECT_PROPERTY_ACTIVITY_REMAINDER_TIME             0xDD5AU
#define MTP_OBJECT_PROPERTY_ACTIVITY_OWNER                      0xDD5BU
#define MTP_OBJECT_PROPERTY_ACTIVITY_STATUS                     0xDD5CU
#define MTP_OBJECT_PROPERTY_OWNER                               0xDD5DU
#define MTP_OBJECT_PROPERTY_EDITOR                              0xDD5EU
#define MTP_OBJECT_PROPERTY_WEBMASTER                           0xDD5FU
#define MTP_OBJECT_PROPERTY_URL_SOURCE                          0xDD60U
#define MTP_OBJECT_PROPERTY_URL_DESTINATION                     0xDD61U
#define MTP_OBJECT_PROPERTY_TIME_BOOKMARK                       0xDD62U
#define MTP_OBJECT_PROPERTY_OBJECT_BOOKMARK                     0xDD63U
#define MTP_OBJECT_PROPERTY_BYTE_BOOKMARK                       0xDD64U
#define MTP_OBJECT_PROPERTY_LAST_BUILD_DATE                     0xDD70U
#define MTP_OBJECT_PROPERTY_TIME_TO_LIVE                        0xDD71U
#define MTP_OBJECT_PROPERTY_MEDIA_GUID                          0xDD72U

/*! @brief MTP device property code */
#define MTP_DEVICE_PROPERTY_UNDEFINED                      0x5000U
#define MTP_DEVICE_PROPERTY_BATTERY_LEVEL                  0x5001U
#define MTP_DEVICE_PROPERTY_FUNCTIONAL_MODE                0x5002U
#define MTP_DEVICE_PROPERTY_IMAGE_SIZE                     0x5003U
#define MTP_DEVICE_PROPERTY_COMPRESSION_SETTING            0x5004U
#define MTP_DEVICE_PROPERTY_WHITE_BALANCE                  0x5005U
#define MTP_DEVICE_PROPERTY_RGB_GAIN                       0x5006U
#define MTP_DEVICE_PROPERTY_F_NUMBER                       0x5007U
#define MTP_DEVICE_PROPERTY_FOCAL_LENGTH                   0x5008U
#define MTP_DEVICE_PROPERTY_FOCUS_DISTANCE                 0x5009U
#define MTP_DEVICE_PROPERTY_FOCUS_MODE                     0x500AU
#define MTP_DEVICE_PROPERTY_EXPOSURE_METERING_MODE         0x500BU
#define MTP_DEVICE_PROPERTY_FLASH_MODE                     0x500CU
#define MTP_DEVICE_PROPERTY_EXPOSURE_TIME                  0x500DU
#define MTP_DEVICE_PROPERTY_EXPOSURE_PROGRAM_MODE          0x500EU
#define MTP_DEVICE_PROPERTY_EXPOSURE_INDEX                 0x500FU
#define MTP_DEVICE_PROPERTY_EXPOSURE_BIAS_COMPENSATION     0x5010U
#define MTP_DEVICE_PROPERTY_DATETIME                       0x5011U
#define MTP_DEVICE_PROPERTY_CAPTURE_DELAY                  0x5012U
#define MTP_DEVICE_PROPERTY_STILL_CAPTURE_MODE             0x5013U
#define MTP_DEVICE_PROPERTY_CONTRAST                       0x5014U
#define MTP_DEVICE_PROPERTY_SHARPNESS                      0x5015U
#define MTP_DEVICE_PROPERTY_DIGITAL_ZOOM                   0x5016U
#define MTP_DEVICE_PROPERTY_EFFECT_MODE                    0x5017U
#define MTP_DEVICE_PROPERTY_BURST_NUMBER                   0x5018U
#define MTP_DEVICE_PROPERTY_BURST_INTERVAL                 0x5019U
#define MTP_DEVICE_PROPERTY_TIMELAPSE_NUMBER               0x501AU
#define MTP_DEVICE_PROPERTY_TIMELAPSE_INTERVAL             0x501BU
#define MTP_DEVICE_PROPERTY_FOCUS_METERING_MODE            0x501CU
#define MTP_DEVICE_PROPERTY_UPLOAD_URL                     0x501DU
#define MTP_DEVICE_PROPERTY_ARTIST                         0x501EU
#define MTP_DEVICE_PROPERTY_COPYRIGHT_INFO                 0x501FU
#define MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER        0xD401U
#define MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME           0xD402U
#define MTP_DEVICE_PROPERTY_VOLUME                         0xD403U
#define MTP_DEVICE_PROPERTY_SUPPORTED_FORMATS_ORDERED      0xD404U
#define MTP_DEVICE_PROPERTY_DEVICE_ICON                    0xD405U
#define MTP_DEVICE_PROPERTY_PLAYBACK_RATE                  0xD410U
#define MTP_DEVICE_PROPERTY_PLAYBACK_OBJECT                0xD411U
#define MTP_DEVICE_PROPERTY_PLAYBACK_CONTAINER_INDEX       0xD412U
#define MTP_DEVICE_PROPERTY_SESSION_INITIATOR_VERSION_INFO 0xD406U
#define MTP_DEVICE_PROPERTY_PERCEIVED_DEVICE_TYPE          0xD407U

/*! @brief MTP operation code */
#define MTP_OPERATION_GET_DEVICE_INFO              0x1001U
#define MTP_OPERATION_OPEN_SESSION                 0x1002U
#define MTP_OPERATION_CLOSE_SESSION                0x1003U
#define MTP_OPERATION_GET_STORAGE_IDS              0x1004U
#define MTP_OPERATION_GET_STORAGE_INFO             0x1005U
#define MTP_OPERATION_GET_NUM_OBJECTS              0x1006U
#define MTP_OPERATION_GET_OBJECT_HANDLES           0x1007U
#define MTP_OPERATION_GET_OBJECT_INFO              0x1008U
#define MTP_OPERATION_GET_OBJECT                   0x1009U
#define MTP_OPERATION_GET_THUMB                    0x100AU
#define MTP_OPERATION_DELETE_OBJECT                0x100BU
#define MTP_OPERATION_SEND_OBJECT_INFO             0x100CU
#define MTP_OPERATION_SEND_OBJECT                  0x100DU
#define MTP_OPERATION_INITIATE_CAPTURE             0x100EU
#define MTP_OPERATION_FORMAT_STORE                 0x100FU
#define MTP_OPERATION_RESET_DEVICE                 0x1010U
#define MTP_OPERATION_SELF_TEST                    0x1011U
#define MTP_OPERATION_SET_OBJECT_PROTECTION        0x1012U
#define MTP_OPERATION_POWER_DOWN                   0x1013U
#define MTP_OPERATION_GET_DEVICE_PROP_DESC         0x1014U
#define MTP_OPERATION_GET_DEVICE_PROP_VALUE        0x1015U
#define MTP_OPERATION_SET_DEVICE_PROP_VALUE        0x1016U
#define MTP_OPERATION_RESET_DEVICE_PROP_VALUE      0x1017U
#define MTP_OPERATION_TERMINATE_OPEN_CAPTURE       0x1018U
#define MTP_OPERATION_MOVE_OBJECT                  0x1019U
#define MTP_OPERATION_COPY_OBJECT                  0x101AU
#define MTP_OPERATION_GET_PARTIAL_OBJECT           0x101BU
#define MTP_OPERATION_INITIATE_OPEN_CAPTURE        0x101CU
#define MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED   0x9801U
#define MTP_OPERATION_GET_OBJECT_PROP_DESC         0x9802U
#define MTP_OPERATION_GET_OBJECT_PROP_VALUE        0x9803U
#define MTP_OPERATION_SET_OBJECT_PROP_VALUE        0x9804U
#define MTP_OPERATION_GET_OBJECT_PROP_LIST         0x9805U
#define MTP_OPERATION_SET_OBJECT_PROP_LIST         0x9806U
#define MTP_OPERATION_GET_INTERDEPENDENT_PROP_DESC 0x9807U
#define MTP_OPERATION_SEND_OBJECT_PROP_LIST        0x9808U
#define MTP_OPERATION_GET_OBJECT_REFERENCES        0x9810U
#define MTP_OPERATION_SET_OBJECT_REFERENCES        0x9811U
#define MTP_OPERATION_SKIP                         0x9820U

/*! @brief MTP response code */
#define MTP_RESPONSE_UNDEFINED                                0x2000U
#define MTP_RESPONSE_OK                                       0x2001U
#define MTP_RESPONSE_GENERAL_ERROR                            0x2002U
#define MTP_RESPONSE_SESSION_NOT_OPEN                         0x2003U
#define MTP_RESPONSE_INVALID_TRANSACTION_ID                   0x2004U
#define MTP_RESPONSE_OPERATION_NOT_SUPPORTED                  0x2005U
#define MTP_RESPONSE_PARAMETER_NOT_SUPPORTED                  0x2006U
#define MTP_RESPONSE_INCOMPLETE_TRANSFER                      0x2007U
#define MTP_RESPONSE_INVALID_STORAGE_ID                       0x2008U
#define MTP_RESPONSE_INVALID_OBJECT_HANDLE                    0x2009U
#define MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED                0x200AU
#define MTP_RESPONSE_INVALID_OBJECT_FORMAT_CODE               0x200BU
#define MTP_RESPONSE_STORAGE_FULL                             0x200CU
#define MTP_RESPONSE_OBJECT_WRITE_PROTECTED                   0x200DU
#define MTP_RESPONSE_STORE_READ_ONLY                          0x200EU
#define MTP_RESPONSE_ACCESS_DENIED                            0x200FU
#define MTP_RESPONSE_NO_THUMBNAIL_PRESENT                     0x2010U
#define MTP_RESPONSE_SELF_TEST_FAILED                         0x2011U
#define MTP_RESPONSE_PARTIAL_DELETION                         0x2012U
#define MTP_RESPONSE_STORE_NOT_AVAILABLE                      0x2013U
#define MTP_RESPONSE_SPECIFICATION_BY_FORMAT_UNSUPPORTED      0x2014U
#define MTP_RESPONSE_NO_VALID_OBJECT_INFO                     0x2015U
#define MTP_RESPONSE_INVALID_CODE_FORMAT                      0x2016U
#define MTP_RESPONSE_UNKNOWN_VENDOR_CODE                      0x2017U
#define MTP_RESPONSE_CAPTURE_ALREADY_TERMINATED               0x2018U
#define MTP_RESPONSE_DEVICE_BUSY                              0x2019U
#define MTP_RESPONSE_INVALID_PARENT_OBJECT                    0x201AU
#define MTP_RESPONSE_INVALID_DEVICE_PROP_FORMAT               0x201BU
#define MTP_RESPONSE_INVALID_DEVICE_PROP_VALUE                0x201CU
#define MTP_RESPONSE_INVALID_PARAMETER                        0x201DU
#define MTP_RESPONSE_SESSION_ALREADY_OPEN                     0x201EU
#define MTP_RESPONSE_TRANSACTION_CANCELLED                    0x201FU
#define MTP_RESPONSE_SPECIFICATION_OF_DESTINATION_UNSUPPORTED 0x2020U
#define MTP_RESPONSE_INVALID_OBJECT_PROP_CODE                 0xA801U
#define MTP_RESPONSE_INVALID_OBJECT_PROP_FORMAT               0xA802U
#define MTP_RESPONSE_INVALID_OBJECT_PROP_VALUE                0xA803U
#define MTP_RESPONSE_INVALID_OBJECT_REFERENCE                 0xA804U
#define MTP_RESPONSE_GROUP_NOT_SUPPORTED                      0xA805U
#define MTP_RESPONSE_INVALID_DATASET                          0xA806U
#define MTP_RESPONSE_SPECIFICATION_BY_GROUP_UNSUPPORTED       0xA807U
#define MTP_RESPONSE_SPECIFICATION_BY_DEPTH_UNSUPPORTED       0xA808U
#define MTP_RESPONSE_OBJECT_TOO_LARGE                         0xA809U
#define MTP_RESPONSE_OBJECT_PROP_NOT_SUPPORTED                0xA80AU

/*! @brief MTP event code */
#define MTP_EVENT_UNDEFINED                 0x4000U
#define MTP_EVENT_CANCEL_TRANSACTION        0x4001U
#define MTP_EVENT_OBJECT_ADDED              0x4002U
#define MTP_EVENT_OBJECT_REMOVED            0x4003U
#define MTP_EVENT_STORE_ADDED               0x4004U
#define MTP_EVENT_STORE_REMOVED             0x4005U
#define MTP_EVENT_DEVICE_PROP_CHANGED       0x4006U
#define MTP_EVENT_OBJECT_INFO_CHANGED       0x4007U
#define MTP_EVENT_DEVICE_INFO_CHANGED       0x4008U
#define MTP_EVENT_REQUEST_OBJECT_TRANSFER   0x4009U
#define MTP_EVENT_STORE_FULL                0x400AU
#define MTP_EVENT_DEVICE_RESET              0x400BU
#define MTP_EVENT_STORAGE_INFO_CHANGED      0x400CU
#define MTP_EVENT_CAPTURE_COMPLETE          0x400DU
#define MTP_EVENT_UNREPORTED_STATUS         0x400EU
#define MTP_EVENT_OBJECT_PROP_CHANGED       0xC801U
#define MTP_EVENT_OBJECT_PROP_DESC_CHANGED  0xC802U
#define MTP_EVENT_OBJECT_REFERENCES_CHANGED 0xC803U

/*! @brief MTP property form flag */
#define MTP_FORM_FLAG_NONE               0x00U
#define MTP_FORM_FLAG_RANGE              0x01U
#define MTP_FORM_FLAG_ENUMERATION        0x02U
#define MTP_FORM_FLAG_DATA_TIME          0x03U
#define MTP_FORM_FLAG_FIXED_LENGTH_ARRAY 0x04U
#define MTP_FORM_FLAG_REGULAR_EXPRESSION 0x05U
#define MTP_FORM_FLAG_BYTE_ARRAY         0x06U
#define MTP_FORM_FLAG_LONG_STRING        0xFFU

/*! @brief MTP storage type */
#define MTP_STORAGE_FIXED_ROM     0x0001U
#define MTP_STORAGE_REMOVABLE_ROM 0x0002U
#define MTP_STORAGE_FIXED_RAM     0x0003U
#define MTP_STORAGE_REMOVABLE_RAM 0x0004U

/*! @brief MTP file system */
#define MTP_STORAGE_UNDEFINED                       0x0000U
#define MTP_STORAGE_FILESYSTEM_GENERIC_FLAT         0x0001U
#define MTP_STORAGE_FILESYSTEM_GENERIC_HIERARCHICAL 0x0002U
#define MTP_STORAGE_FILESYSTEM_DCF                  0x0003U

/*! @brief MTP access capbility */
#define MTP_STORAGE_READ_WRITE               0x0000U
#define MTP_STORAGE_READ_ONLY_WITHOUT_DELETE 0x0001U
#define MTP_STORAGE_READ_ONLY_WITH_DELETE    0x0002U

/*! @}*/

#endif
