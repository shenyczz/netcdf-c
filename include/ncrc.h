/*
Copyright (c) 1998-2017 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/

/*
Common functionality for reading
and accessing rc files (e.g. .daprc).
*/

#ifndef NCRC_H
#define NCRC_H

/* Need these support includes */
#include "ncuri.h"
#include "nclist.h"
#include "ncbytes.h"

typedef struct NCTriple {
	char* host; /* combined host:port */
        char* key;
        char* value;
} NCTriple;

/* collect all the relevant info around the rc file */
typedef struct NCRCinfo {
	int ignore; /* if 1, then do not use any rc file */
	int loaded; /* 1 => already loaded */
        NClist* triples; /* the rc file triple store fields*/
        char* rcfile; /* specified rcfile; overrides anything else */
} NCRCinfo;

/* Collect global state info in one place */
typedef struct NCRCglobalstate {
    int initialized;
    char* tempdir; /* track a usable temp dir */
    char* home; /* track $HOME for use in creating $HOME/.oc dir */
    NCRCinfo rcinfo; /* Currently only one rc file per session */
} NCRCglobalstate;

/* Define a structure to hold all legal .daprc fields */
/* For char*, NULL means it was not defined, for int and long,
   -1 means it was not defined. KeepAlive.on == -1 indicates not defined.
*/
typedef struct NCRCFIELDS {
    int   HTTP_VERBOSE;
    int   HTTP_DEFLATE;
    char* HTTP_COOKIEJAR;
    char* HTTP_CREDENTIALS_USERNAME;
    char* HTTP_CREDENTIALS_PASSWORD;
    char* HTTP_SSL_CERTIFICATE;
    char* HTTP_SSL_KEY;
    char* HTTP_SSL_KEYPASSWORD;
    char* HTTP_SSL_CAPATH;
    int   HTTP_SSL_VALIDATE;
    long  HTTP_TIMEOUT;
    char* HTTP_PROXY_SERVER;
    long  HTTP_READ_BUFFERSIZE;
    char* HTTP_NETRC;
    char* HTTP_USERAGENT;
    struct KeepAlive {int defined; int active; int wait; int repeat;}
           HTTP_KEEPALIVE;
} NCRCFIELDS;

extern NCRCglobalstate ncrc_globalstate; /* singleton instance */

extern const NCRCFIELDS NC_dfaltfields;

/* From drc.c */
/* read and compile the rc file, if any */
extern int NC_rcload(void);
extern char* NC_rclookup(const char* key, const char* hostport);
extern void NC_rcclear(NCRCinfo* info);
extern int NC_set_rcfile(const char* rcfile);
extern int NC_rcfile_insert(const char* key, const char* value, const char* hostport);
extern int NC_rcloadfields(NCRCFIELDS* fields, const char* hostport);
extern int NC_rcloadfield(NCRCFIELDS* fields, const char* key, const char* value);

/* From dutil.c (Might later move to e.g. nc.h */
extern int NC__testurl(const char* path, char** basenamep);
extern int NC_isLittleEndian(void);
extern char* NC_backslashEscape(const char* s);
extern char* NC_backslashUnescape(const char* esc);
extern char* NC_entityescape(const char* s);
extern int NC_readfile(const char* filename, NCbytes* content);
extern int NC_writefile(const char* filename, size_t size, void* content);
extern char* NC_mktmp(const char* base);

#endif /*NCRC_H*/
