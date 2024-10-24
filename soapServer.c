/* soapServer.c
   Generated by gSOAP 2.8.133 for conecta4.h

gSOAP XML Web services tools
Copyright (C) 2000-2024, Robert van Engelen, Genivia Inc. All Rights Reserved.
The soapcpp2 tool and its generated software are released under the GPL.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
--------------------------------------------------------------------------------
A commercial use license is available from Genivia Inc., contact@genivia.com
--------------------------------------------------------------------------------
*/

#if defined(__BORLANDC__)
#pragma option push -w-8060
#pragma option push -w-8004
#endif
#include "soapH.h"

SOAP_SOURCE_STAMP("@(#) soapServer.c ver 2.8.133 2024-10-24 14:45:43 GMT")
SOAP_FMAC5 int SOAP_FMAC6 soap_serve(struct soap *soap)
{
#ifndef WITH_FASTCGI
	soap->keep_alive = soap->max_keep_alive + 1;
#endif
	do
	{
#ifndef WITH_FASTCGI
		if (soap->keep_alive > 0 && soap->max_keep_alive > 0)
			soap->keep_alive--;
#endif
		if (soap_begin_serve(soap))
		{	if (soap->error >= SOAP_STOP)
				continue;
			return soap->error;
		}
		if ((soap_serve_request(soap) || (soap->fserveloop && soap->fserveloop(soap))) && soap->error && soap->error < SOAP_STOP)
		{
#ifdef WITH_FASTCGI
			soap_send_fault(soap);
#else
			return soap_send_fault(soap);
#endif
		}
#ifdef WITH_FASTCGI
		soap_destroy(soap);
		soap_end(soap);
	} while (1);
#else
	} while (soap->keep_alive);
#endif
	return SOAP_OK;
}

#ifndef WITH_NOSERVEREQUEST
SOAP_FMAC5 int SOAP_FMAC6 soap_serve_request(struct soap *soap)
{
	(void)soap_peek_element(soap);
	if (!soap_match_tag(soap, soap->tag, "conecta4ns:register"))
		return soap_serve_conecta4ns__register(soap);
	if (!soap_match_tag(soap, soap->tag, "conecta4ns:getStatus"))
		return soap_serve_conecta4ns__getStatus(soap);
	return soap->error = SOAP_NO_METHOD;
}
#endif

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_conecta4ns__register(struct soap *soap)
{	struct conecta4ns__register soap_tmp_conecta4ns__register;
	struct conecta4ns__registerResponse soap_tmp_conecta4ns__registerResponse;
	int soap_tmp_int;
	soap_default_conecta4ns__registerResponse(soap, &soap_tmp_conecta4ns__registerResponse);
	soap_default_int(soap, &soap_tmp_int);
	soap_tmp_conecta4ns__registerResponse.code = &soap_tmp_int;
	soap_default_conecta4ns__register(soap, &soap_tmp_conecta4ns__register);
	if (!soap_get_conecta4ns__register(soap, &soap_tmp_conecta4ns__register, "conecta4ns:register", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = conecta4ns__register(soap, soap_tmp_conecta4ns__register.playerName, soap_tmp_conecta4ns__registerResponse.code);
	if (soap->error)
		return soap->error;
	soap->encodingStyle = ""; /* use SOAP encoding style */
	soap_serializeheader(soap);
	soap_serialize_conecta4ns__registerResponse(soap, &soap_tmp_conecta4ns__registerResponse);
	if (soap_begin_count(soap))
		return soap->error;
	if ((soap->mode & SOAP_IO_LENGTH))
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_conecta4ns__registerResponse(soap, &soap_tmp_conecta4ns__registerResponse, "conecta4ns:registerResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_conecta4ns__registerResponse(soap, &soap_tmp_conecta4ns__registerResponse, "conecta4ns:registerResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_conecta4ns__getStatus(struct soap *soap)
{	struct conecta4ns__getStatus soap_tmp_conecta4ns__getStatus;
	struct conecta4ns__getStatusResponse soap_tmp_conecta4ns__getStatusResponse;
	struct tBlock soap_tmp_conecta4ns__tBlock;
	soap_default_conecta4ns__getStatusResponse(soap, &soap_tmp_conecta4ns__getStatusResponse);
	soap_default_conecta4ns__tBlock(soap, &soap_tmp_conecta4ns__tBlock);
	soap_tmp_conecta4ns__getStatusResponse.status = &soap_tmp_conecta4ns__tBlock;
	soap_default_conecta4ns__getStatus(soap, &soap_tmp_conecta4ns__getStatus);
	if (!soap_get_conecta4ns__getStatus(soap, &soap_tmp_conecta4ns__getStatus, "conecta4ns:getStatus", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = conecta4ns__getStatus(soap, soap_tmp_conecta4ns__getStatus.playerName, soap_tmp_conecta4ns__getStatus.gameId, soap_tmp_conecta4ns__getStatusResponse.status);
	if (soap->error)
		return soap->error;
	soap->encodingStyle = ""; /* use SOAP encoding style */
	soap_serializeheader(soap);
	soap_serialize_conecta4ns__getStatusResponse(soap, &soap_tmp_conecta4ns__getStatusResponse);
	if (soap_begin_count(soap))
		return soap->error;
	if ((soap->mode & SOAP_IO_LENGTH))
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_conecta4ns__getStatusResponse(soap, &soap_tmp_conecta4ns__getStatusResponse, "conecta4ns:getStatusResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_conecta4ns__getStatusResponse(soap, &soap_tmp_conecta4ns__getStatusResponse, "conecta4ns:getStatusResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

#if defined(__BORLANDC__)
#pragma option pop
#pragma option pop
#endif

/* End of soapServer.c */
