#ifndef _MISC_H_
#define _MISC_H_

#define MD5_MAX_LEN 33

int MD5_File(const char *pcFilePath, char *pcMd5);
int MD5_Bin(const unsigned char *pucData, const unsigned int uiDataLen, char *pcMd5);

char *base64_encode(const unsigned char * bindata, int binlength, char * base64);
int base64_decode( const char * base64, unsigned char * bindata );

#endif	/* _MISC_H_ */
