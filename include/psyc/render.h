#ifndef PSYC_RENDER_H
#define PSYC_RENDER_H

#include "packet.h"

/**
 * @file psyc/render.h
 * @brief Interface for PSYC packet rendering.
 *
 * All rendering functions and the definitions they use are defined here.
 */

/**
 * @defgroup render Rendering Functions
 *
 * This module contains all rendering functions.
 * @{
 */

/**
 * Return codes for psyc_render.
 */
typedef enum {
    /// Error, method is missing, but data is present.
    PSYC_RENDER_ERROR_METHOD_MISSING = -3,
    /// Error, a modifier name is missing.
    PSYC_RENDER_ERROR_MODIFIER_NAME_MISSING = -2,
    /// Error, buffer is too small to render the packet.
    PSYC_RENDER_ERROR = -1,
    /// Packet is rendered successfully in the buffer.
    PSYC_RENDER_SUCCESS = 0,
} PsycRenderRC;

/**
 * Render a PSYC packet into a buffer.
 *
 * The packet structure should contain the packet parts, either routing, entity,
 * method & data, or routing & content when rendering raw content.
 * It should also contain the contentLength & total length of the packet,
 * you can use psyc_packet_length_set() for calculating & setting these values.
 * This function renders packet->length bytes to the buffer,
 * if buflen is less than that an error is returned.
 *
 * @see psyc_packet_init()
 * @see psyc_packet_init_raw()
 * @see psyc_packet_length_set()
 */
#ifdef __INLINE_PSYC_RENDER
static inline
#endif
PsycRenderRC
psyc_render (PsycPacket *packet, char *buffer, size_t buflen);

size_t
psyc_render_modifier (PsycModifier *mod, char *buffer);

PsycRenderRC
psyc_render_elem (PsycElem *elem, char *buffer, size_t buflen);

PsycRenderRC
psyc_render_dict_key (PsycDictKey *elem, char *buffer, size_t buflen);

/**
 * Render a PSYC list into a buffer.
 */
#ifdef __INLINE_PSYC_RENDER
static inline
#endif
PsycRenderRC
psyc_render_list (PsycList *list, char *buffer, size_t buflen);

PsycRenderRC
psyc_render_dict (PsycDict *dict, char *buffer, size_t buflen);

/** @} */ // end of render group

#endif
