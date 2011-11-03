#include "lib.h"
#include <psyc/syntax.h>
#include <psyc/packet.h>

inline
PsycListFlag psyc_list_length_check (PsycList *list)
{
	PsycListFlag flag = PSYC_LIST_NO_LENGTH;
	size_t i, length = 0;

	for (i = 0; i < list->num_elems; i++)
	{
		PsycString *elem = &list->elems[i];
		length += 1 + elem->length; // |elem
		if (length > PSYC_MODIFIER_SIZE_THRESHOLD ||
				memchr(elem->data, (int)'|', elem->length) ||
				memchr(elem->data, (int)'\n', elem->length))
		{
			flag = PSYC_LIST_NEED_LENGTH;
			break;
		}
	}

	return flag;
}

inline
PsycListFlag psyc_list_length (PsycList *list)
{
	size_t i, length = 0;

	if (list->flag == PSYC_LIST_NEED_LENGTH)
	{
		for (i = 0; i < list->num_elems; i++)
		{
			if (i > 0)
				length++; // |
			length += psyc_num_length(list->elems[i].length) + 1 + list->elems[i].length; // length SP elem
		}
	}
	else
	{
		for (i = 0; i < list->num_elems; i++)
			length += 1 + list->elems[i].length; // |elem
	}

	return length;
}

inline
void psyc_list_init (PsycList *list, PsycString *elems, size_t num_elems,
                     PsycListFlag flag)
{
	*list = (PsycList) {num_elems, elems, 0, flag};

	if (flag == PSYC_LIST_CHECK_LENGTH) // check if list elements need length
		list->flag = psyc_list_length_check(list);

	list->length = psyc_list_length(list);
}


inline
size_t psyc_modifier_length (PsycModifier *m)
{
	size_t length = 1 + // oper
		m->name.length + 1 + // name\t
		m->value.length + 1; // value\n

	if (m->flag == PSYC_MODIFIER_NEED_LENGTH) // add length of length if needed
		length += psyc_num_length(m->value.length) + 1; // SP length

	return length;
}

inline
PsycPacketFlag psyc_packet_length_check (PsycPacket *p)
{
	if (p->data.length == 1 && p->data.data[0] == C_GLYPH_PACKET_DELIMITER)
		return PSYC_PACKET_NEED_LENGTH;

	if (p->data.length > PSYC_CONTENT_SIZE_THRESHOLD)
		return PSYC_PACKET_NEED_LENGTH;

	int i;
	// if any entity modifiers need length it is possible they contain
	// a packet terminator, thus the content should have a length as well
	for (i = 0; i < p->entity.lines; i++)
		if (p->entity.modifiers[i].flag == PSYC_MODIFIER_NEED_LENGTH)
			return PSYC_PACKET_NEED_LENGTH;

	if (memmem(p->data.data, p->data.length, PSYC_C2ARG(PSYC_PACKET_DELIMITER)))
		return PSYC_PACKET_NEED_LENGTH;

	return PSYC_PACKET_NO_LENGTH;
}

inline
size_t psyc_packet_length_set (PsycPacket *p)
{
	size_t i;
	p->routingLength = 0;
	p->contentLength = 0;

	// add routing header length
	for (i = 0; i < p->routing.lines; i++)
		p->routingLength += psyc_modifier_length(&(p->routing.modifiers[i]));

	if (p->content.length)
		p->contentLength = p->content.length;
	else
	{
		// add entity header length
		for (i = 0; i < p->entity.lines; i++)
			p->contentLength += psyc_modifier_length(&(p->entity.modifiers[i]));

		// add length of method, data & delimiter
		if (p->method.length)
			p->contentLength += p->method.length + 1; // method\n
		if (p->data.length)
			p->contentLength += p->data.length + 1; // data\n
	}

	// set total length: routing-header content |\n
	p->length = p->routingLength + p->contentLength + 2;

	if (p->contentLength > 0 || p->flag == PSYC_PACKET_NEED_LENGTH)
		p->length++; // add \n at the start of the content part

	if (p->flag == PSYC_PACKET_NEED_LENGTH) // add length of length if needed
		p->length += psyc_num_length(p->contentLength);

	return p->length;
}

inline
void psyc_packet_init (PsycPacket *p,
                       PsycModifier *routing, size_t routinglen,
                       PsycModifier *entity, size_t entitylen,
                       char *method, size_t methodlen,
                       char *data, size_t datalen,
                       PsycPacketFlag flag)
{
	*p = (PsycPacket) {{routinglen, routing}, {entitylen, entity},
	                   {methodlen, method}, {datalen, data}, {0,0}, 0, 0, flag};

	if (flag == PSYC_PACKET_CHECK_LENGTH) // find out if it needs length
		p->flag = psyc_packet_length_check(p);

	psyc_packet_length_set(p);
}

inline
void psyc_packet_init_raw (PsycPacket *p,
                           PsycModifier *routing, size_t routinglen,
                           char *content, size_t contentlen,
                           PsycPacketFlag flag)
{
	*p = (PsycPacket) {{routinglen, routing}, {0,0}, {0,0}, {0,0},
	                   {contentlen, content}, 0, 0, flag};

	if (flag == PSYC_PACKET_CHECK_LENGTH) // find out if it needs length
		p->flag = psyc_packet_length_check(p);

	psyc_packet_length_set(p);
}
