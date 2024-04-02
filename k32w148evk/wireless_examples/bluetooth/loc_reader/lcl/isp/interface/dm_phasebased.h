/*
 * Copyright 2019-2020, 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @addtogroup GENFSK_Localization
 * @addtogroup PhaseBaseDistEstimate Phase-based Distance Estimation localization feature
 * @ingroup GENFSK_Localization
 * @{
 */

/*! @file
 * GENFSK Localization - Phase-based Distance Estimation using complex domain estimation (CDE).
 */

#include <stdio.h>
#include <stdbool.h>
#include "arm_math.h"

#ifndef DM_PHASEBASED_H

#define DM_PHASEBASED_H

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! algorithms default configuration */
#define DM_CDE_THRESHOLD_UNDEF                (0U)
#define DM_CDE_THRESHOLD_DEFAULT              (6226U)  /* 0.38 in Q14 */
#define DM_CDE_THRESHOLD_DIVERSITY_DEFAULT    (10650U) /* 0.65 in Q14 */

#define DM_SRDE_L_DEFAULT                     (40U)
#define DM_SRDE_METHOD_SUBSPACE_DEFAULT       (8U)
#define DM_SRDE_TOL2_DEFAULT                  (990U) /* x1000 (0.99) */
#define DM_SRDE_MAX_ITER_DEFAULT              (200U)

/* Constants */
#define C_LIGHT    (300000000)
#define DM_FREQUENCY_RASTER   (1000U) /* in kHz */
#define Q7_SCALING_FACTOR     (128)
#define Q10_SCALING_FACTOR    (1024)
#define Q14_SCALING_FACTOR    (16384)
#define Q15_SCALING_FACTOR    (32768)

#define DM_MAX_FREQ_NB (128U) /*!< Maximum number of frequencies that can be passed to the API */
#define N_FFT_MAX      (256U) /*!< Maximum size of the FFT that can be requested to the API */

#define DM_MAX_IQ_CODES (2048U) /*!< IQs are Q10 */

#if N_FFT_MAX < DM_MAX_FREQ_NB
#error Maximum FFT size shall be larger or equal than number of frequencies
#endif

/*******************************************************************************
 * Types
 ******************************************************************************/

/*!
 * API return codes
 */
typedef enum {
    DM_STATUS_OK,
    DM_STATUS_INVALID_PARAMETER,
    DM_STATUS_INVALID_DISTANCE,
    DM_STATUS_INVALID_DQI,
    DM_STATUS_MALLOC_FAIL
} dm_status_t;

/*!
 * @brief Distance measurement output for CDE method.
 */
typedef struct
{
    int              distance;          /*!< Distance computed by the present algorithm in Q21.10 format */
    int              distance_interp;   /*!< Distance computed by the interpolation variant of present algorithm in Q21.10 format */
    unsigned short   d_fft_idx;         /*!< IFFT index chosen by the algorithm. Range is [0; N_FFT-1]. */
    q15_t            dqi;               /*!< Raw distance quality indicator metric in Q2.14 format. */
    unsigned char    is_valid;          /*!< Boolean flag to indicate validity of this estimate. */
} dm_cde_out_t;

/*!
 * @brief Distance estimation from CDE interpolator.
 */
typedef struct dm_cde_estimate_tag
{
    int32_t distance_estimate;   /*!< Estimated distance in Q21.10 format */
    int16_t dqi;                 /*!< Distance quality indicator metric in Q2.14 format */
} dm_cde_estimate_t;

/*!
 * @brief Distance measurement output for CDE method.
 */
typedef struct
{
    int              distance;          /*!< Distance computed by the present algorithm in Q16.15 format */
    q15_t            dqi;               /*!< Distance quality indicator metric in Q15 format. */
} dm_slope_out_t;

/*!
 * @brief Distance measurement debug information output for CDE method.
 */
typedef struct
{
    q15_t            *fft_mag_p;        /*!< Pointer to IFFT magnitude array. Values are Q2.14. Array size is N_FFT. */
} dm_cde_debug_info_t;

/*!
 * @brief Distance measurement debug information output for slope-based method.
 */
typedef struct
{
    q15_t            slope;             /*!< Computed slope in Q15. */
} dm_slope_debug_info_t;

/*!
 * @brief Frequency validity mask.
 * mask[0] : b31 (MSB) ............... b0 (LSB)
 * mask[0] : Fstart + 31*freq_step ... Fstart
 * etc...
 * '0' means that phase value for corresponding frequency in input vector is invalid
 * '1' means that phase value for corresponding frequency in input vector is valid
 * unused frequency must be padded with zeros
 */
typedef uint32_t dm_freq_mask_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief Phase based distance measurement algorithm.
 *
 * Use complex-domain estimation.
 * This function can be called either with:
 *  - combined Phi vector input (kept for backward compatibility)
 *  - or raw IQ vectors (recommended)
 * as input agruments
 *
 * @param nb_meas          Number of entries in phi_fp vector, corresponding to the number of used frequencies (input)
 * @param phi_fp           Pointer on Phi vector array as defined in the theory. (input)
                           Phase values (phi_fp[]) are expected to be scaled to [-1;1[ in Q15 fixed-point format
                           When phi_fp is provided,  iq1 and iq2 must be set to NULL
*  @param iq1              Pointer on IQ vector array mesured on one side. (input)
*  @param iq1              Pointer on IQ vector array mesured on the other side. (input)
                           IQ vectors are expected with I and Q interleaved (I first) with I and Q values in [-2048;2047] range
                           (encoded on 11bits + one sign bit extended to int16_t)
                           When iq1 and iq2 are provided, phi_fp must be set to NULL
 * @param freq_mask        Pointer on Frequency validity mask (input)
 * @param freq_step        Frequency step in KHz between 2 consecutive frequencies (input)
 * @param n_fft            IFFT size. Allowed values are 64, 128 and 256 (recommended) (input)
 * @param threshold        IFFT magnitude threshold in 0.001 steps. Typical value is 200 (0.2) - deprecated, set to zero
 * @param nb_est           Number of distance estimations required from the algorithm. Also size of out_p as provided by the caller.
 * @param out_p            Pointer on array of estimate (size nb_est)
 * @param dbg_info_p       Pointer on additional debug info structure. May be NULL. (output)
 *
 * @retval dm_status_t
 *
 */
dm_status_t dm_complex_domain_based_fp(const unsigned short nb_meas,
                                       const q15_t phi_fp[],
                                       const int16_t iq1[],
                                       const int16_t iq2[],
                                       const dm_freq_mask_t *freq_mask,
                                       const unsigned short freq_step,
                                       const unsigned short n_fft,
                                       const unsigned short threshold,
                                       const unsigned char  nb_est,
                                       dm_cde_out_t         out_p[]
                                       );

/*!
 * @brief Phase based distance measurement algorithm.
 *
 * Slope-based estimation:
 *
 * @param nb_meas          Number of entries in phi_fp vector, corresponding to the number of used frequencies (input)
 * @param phi_fp           Pointer on Phi vector array as defined in the theory. (input)
                           Phase values (phi_fp[]) are expected to be scaled to [-1;1[ in Q15 fixed-point format
 * @param freq_mask        Pointer on Frequency validity mask (input)
 * @param freq_step_khz    Frequency step in KHz between 2 consecutive frequencies (input)
 * @param out_p            Pointer on structure for storing estimation result.
 * @param dbg_info_p       Pointer on additional debug info structure. May be NULL. (output)
 *
 * @retval dm_status_t
 *
 */
dm_status_t dm_slope_based_fp(const unsigned short nb_meas,
                              const q15_t phi_fp[],
                              const dm_freq_mask_t *freq_mask,
                              const unsigned short f_step_khz,
                              dm_slope_out_t *out_p,
                              dm_slope_debug_info_t *dbg_info_p
                              );

                                                                                       /*!
 * @brief Phase based distance measurement algorithm.
 *
 * Final CDE DQI computation based on IFFT magnitude output, scaled to [0;1]
 *
 * @param freq_step        Frequency step in KHz between 2 consecutive frequencies (input)
 * @param n_fft            IFFT size. Allowed values are 64, 128 and 256 (recommended) (input)
 * @param cde_estimate_p   Pointer on selected estimate (input)
 *
 * @retval Final DQI in Q14 format
 *
 */
q15_t dm_cde_compute_final_dqi(  const unsigned short freq_step, const unsigned short n_fft, dm_cde_out_t *cde_estimate_p);

/*!
 * Return distance estimation for a given input vector and frequency mask.
 * This methode implements frequency slope estimation and CDE algorithm.
 * Final distance estimation is performed using IFFT magnitude threshold from CDE or a combination of frequency slope estimation and CDE algorithm.

 * In case pIQin1 & 2 buffers are provided, the PhaseVector will be used to output the combined phase
 * In case pIQin1 & 2 buffers are not provided (set to NULL), the PhaseVector will be used as an intput
 * and shall convey the combined phase
 *
 *  @param [in]  pIQin1          Pointer on IQ vector array mesured on one side. (input)
 *  @param [in]  pIQin2          Pointer on IQ vector array mesured on the other side. (input)
 *                               IQ vectors are expected with I and Q interleaved (I first) with I and Q values in [-2048;2047] range
 *                               (encoded on 11bits + one sign bit extended to int16_t)
 * @param [in/out]               PhaseVector  Pointer to phase vector (one phase per frequency)
 * @param [in] FreqMask          Pointer to frequency mask (one bit per frequency, 1=valid)
 * @param [in] NumberFreqs       Number of frequencies in the vector
 * @param [in] FreqStep          Frequency interval between two entries in the PhaseVector
 * @param [in] ZeroMComp         Zero-meters compensation value, Q10 format.
 * @param [in] threshold         IFFT magnitude threshold to the maximum peak magintude in Q14 format. Recommended value is 6226 (0.38)
 *                               In case threshold is set to zero, no IFFT threshold will be applied and slope estimation will be used
 *                               to refine IFFT peak selection
 * @param [out] ChosenEstimate   Pointer to output structure, will contain distance in meters and the Final DQI
 * @see dm_cde_out_t, dm_complex_domain_based_fp
 * @retval measureIsValid        TRUE if filters did validate the measurement, FALSE if filters detected some inconsistency.

 */
bool dm_cde_distance_estimation(int16_t *pIQin1, int16_t *pIQin2, int16_t *PhaseVector,
                                dm_freq_mask_t *FreqMask, uint16_t NumberFreqs, uint16_t FreqStep,
                                int32_t ZeroMComp, uint32_t threshold, dm_cde_estimate_t *ChosenEstimate);


/*!
 * Returns the CDE distance estimate for the selected index.
 * Note that DQI populated by this function is the raw DQI
 *
 * @param [out] pEstimateResult Pointer to the distance estimate structure for the given index
 * @param [in]  index           Index for the estimate to extract. index must be < LCL_PDE_POSTPROC_MAX_ESTIMATES.
 *
 * @retval Status               0 if executed correctly, 1 if failure occurred.
 */
bool dm_cde_get_estimate_by_index(dm_cde_estimate_t* pEstimateResult, uint16_t index);

#define IS_FREQ_SMP_VALID(index, mask) ((mask == NULL) || (((mask[(index) / 32] >> ((index) % 32)) & 0x1) == 1U))

/*******************************************************************************
 * Types
 ******************************************************************************/
/*!
 * @brief Distance estimation from SRDE.
 */
typedef struct
{
    float_t distance_estimate;   /*!< Estimated distance */
    float_t dqi;                 /*!< Distance quality indicator metric  (likeliness) */
}dm_srde_estimate_t;
/*!
 * @brief Conifguration SRDE method.
 */
typedef struct
{
    uint8_t  maxiter;  /*!< Number of max iteration that the EVD algorithm is able to do  */
    uint8_t  methodSubspaceSeparation; /*!< Subspace separation value */
    float_t  TOL2; /*!<  */
    uint8_t  L; /*!< Covariance matrix size  */
} dm_srde_config;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief Phase based distance measurement algorithm.
 *
 * Return distance estimation for a given input vector and frequency mask.
 * This method implements SRDE algorithm.
*
*  @param [in]  pIQin1                        Pointer on IQ vector array measured on one side. (input)
*  @param [in]  pIQin2                        Pointer on IQ vector array measured on the other side. (input)
*                                               IQ vectors are expected with I and Q interleaved (I first) with I and Q values in [-1024;1023] range
*                                               (encoded on 10bits + one sign bit extended to int16_t)
* @param [in] FreqMask                        Pointer to frequency mask (one bit per frequency, 1=valid)
* @param [in] n_ap                            Number of antenna paths
* @param [in] NumberFreqs                     Number of frequencies in the input vectors
* @param [in] FreqStep                        Frequency separation in KHz
* @param [in] config                          Pointer to input structure, will contain algorithm configuration parameters, if pointers is NULL then default parameters are used
* @param [out] DistanceEstimate               Pointer to output structure, will contain distance in meters and the Final DQI
* @retval dm_status_t
 */
#ifdef __cplusplus
extern "C" {
#endif
dm_status_t dm_srde_distance_estimation(int16_t *pIQin1, int16_t *pIQin2, dm_freq_mask_t *FreqMask, uint16_t n_ap, uint16_t NumberFreqs, uint16_t FreqStep, dm_srde_config *config, dm_srde_estimate_t *DistanceEstimate);
#ifdef __cplusplus
};
#endif

#endif /* DM_PHASEBASED_H */

/*! @} */
