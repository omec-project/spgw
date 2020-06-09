// SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef __GTPv2_PARSING_HELPER
#define __GTPv2_PARSING_HELPER
/**
 * @brief  : Helper macro to calculate the address of some offset from some base address
 * @param  : base, base or starting address
 * @param  : offset, offset to be added to base for return value
 * @return : Cacluated address of Offset from some Base address
 */
#define IE_LIMIT(base, offset) \
	(gtpv2c_ie *)((uint8_t *)(base) + offset)

/**
 * @brief  : Helper macro to calculate the limit of a Gropued Information Element
 * @param  : gtpv2c_ie_ptr
 *           Pointer to address of a Grouped Information Element
 * @return : The limit (or exclusive end) of a grouped information element by its length field
 */
#define GROUPED_IE_LIMIT(gtpv2c_ie_ptr)\
	IE_LIMIT(gtpv2c_ie_ptr + 1, ntohs(gtpv2c_ie_ptr->length))

/**
 * @brief  : Helper macro to calculate the limit of a GTP message buffer given the GTP
 *           header (which contains its length)
 * @param  : gtpv2c_h
 *           Pointer to address message buffer containing a GTP Header
 * @return : The limit (or exclusive end) of a GTP message (and thus its IEs) given the
 *           message buffer containing a GTP header and its length field.
 */
#define GTPV2C_IE_LIMIT(gtpv2c_h)\
	IE_LIMIT(&gtpv2c_h->teid, ntohs(gtpv2c_h->gtpc.message_len))

/**
 * @brief  : Helper function to get the location, according to the buffer and gtp header
 *           located at '*gtpv2c_h', of the first information element according to
 *           3gppp 29.274 clause 5.6, & figure 5.6-1
 * @param  : gtpv2c_h
 *           header and buffer containing gtpv2c message
 * @return : - NULL \- No such information element exists due to address exceeding limit
 *           - pointer to address of first information element, if exists.
 */
gtpv2c_ie *
get_first_ie(gtpv2c_header_t * gtpv2c_h);

/**
 * @brief  : Helper macro to loop through GTPv2C Information Elements (IE)
 * @param  : gtpv2c_h
 *           Pointer to address message buffer containing a GTP Header
 * @param  : gtpv2c_ie_ptr
 *           Pointer to starting IE to loop from
 * @param  : gtpv2c_limit_ie_ptr
 *           Pointer to ending IE of the loop
 * @return : nothing
 *
 */
#define FOR_EACH_GTPV2C_IE(gtpv2c_h, gtpv2c_ie_ptr, gtpv2c_limit_ie_ptr) \
	for (gtpv2c_ie_ptr = get_first_ie(gtpv2c_h),                 \
		gtpv2c_limit_ie_ptr = GTPV2C_IE_LIMIT(gtpv2c_h);         \
		gtpv2c_ie_ptr;                                           \
		gtpv2c_ie_ptr = get_next_ie(gtpv2c_ie_ptr, gtpv2c_limit_ie_ptr))

/**
 * @brief  : Calculates address of Information Element which follows gtpv2c_ie_ptr
 *           according to its length field while considering the limit, which may be
 *           calculated according to the buffer allocated for the GTP message or length of
 *           a Information Element Group
 *
 * @param  : gtpv2c_ie_ptr
 *           Known information element preceding desired information element.
 * @param  : limit
 *           Memory limit for next information element, if one exists
 * @return : - NULL \- No such information element exists due to address exceeding limit
 *           - pointer to address of next available information element
 */
gtpv2c_ie *
get_next_ie(gtpv2c_ie *gtpv2c_ie_ptr, gtpv2c_ie *limit);

/**
 * @brief  : Helper macro to loop through GTPv2C Grouped Information Elements (IE)
 * @param  : parent_ie_ptr
 *           Pointer to address message buffer containing a parent GTPv2C IE
 * @param  : child_ie_ptr
 *           Pointer to starting child IE to loop from
 * @param  : gtpv2c_limit_ie_ptr
 *           Pointer to ending IE of the loop
 * @return : Nothing
 *
 */
#define FOR_EACH_GROUPED_IE(parent_ie_ptr, child_ie_ptr, gtpv2c_limit_ie_ptr) \
	for (gtpv2c_limit_ie_ptr = GROUPED_IE_LIMIT(parent_ie_ptr),           \
	       child_ie_ptr = parent_ie_ptr + 1;                              \
	       child_ie_ptr;                                                  \
	       child_ie_ptr = get_next_ie(child_ie_ptr, gtpv2c_limit_ie_ptr))

/**
 * @brief  : Macro to provide address of first Information Element within message buffer
 *           containing GTP header. Address may be invalid and must be validated to ensure
 *           it does not exceed message buffer.
 * @param  : gtpv2c_h
 *           Pointer of address of message buffer containing GTP header.
 * @return : Pointer of address of first Information Element.
 */
#define IE_BEGIN(gtpv2c_h)                               \
	  ((gtpv2c_h)->gtpc.teid_flag                              \
	  ? (gtpv2c_ie *)((&(gtpv2c_h)->teid.has_teid)+1)      \
	  : (gtpv2c_ie *)((&(gtpv2c_h)->teid.no_teid)+1))

/**
 * @brief  : Macro to provide address of next Information Element within message buffer
 *           given previous information element. Address may be invalid and must be
 *           validated to ensure it does not exceed message buffer.
 * @param  : gtpv2c_ie_ptr
 *           Pointer of address of information element preceding desired IE..
 * @return : Pointer of address of following Information Element.
 */
#define NEXT_IE(gtpv2c_ie_ptr) \
	(gtpv2c_ie *)((uint8_t *)(gtpv2c_ie_ptr + 1) \
	+ ntohs(gtpv2c_ie_ptr->length))



#endif
