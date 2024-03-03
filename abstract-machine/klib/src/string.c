#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  //panic("Not implemented");
  //assert(0);
  const char *ptr=s;
  while(*ptr!='\0'){
    ptr++;
  }
  //assert(0);
  return ptr-s;
}

char *strcpy(char *dst, const char *src) {
  //panic("Not implemented");
    char * cp = dst;
        while( (*cp++ = *src++) )
                ;               /* Copy src over dst */
        return( dst );
}

char *strncpy(char *dst, const char *src, size_t n) {
  //panic("Not implemented");
  char *cp=dst;
  size_t size=0;
  for(size=0;size<n&&src[size]!='\0';size++){
    cp[size]=src[size];
  }
  //assert(0);
  for(;size<n;size++){
    cp[size]='\0';
  }
  //assert(0);
  return cp;
}

char *strcat(char *dst, const char *src) {
  //panic("Not implemented");
    char * cp = dst;
 
    while( *cp )
      cp++; /* find end of dst */
    while( (*cp++ = *src++) ) ; /* Copy src to end of dst */
    return( dst ); /* return dst */

}

int strcmp(const char *s1, const char *s2) {
  //panic("Not implemented");
    const unsigned char *p1 = (const unsigned char *) s1;
    const unsigned char *p2 = (const unsigned char *) s2;
    unsigned char c1, c2;

    do {
        c1 = (unsigned char) *p1++;
        c2 = (unsigned char) *p2++;
        if(c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);

    return c1 - c2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  //panic("Not implemented");
  const char *p1=s1;
  const char *p2=s2;
  for(;n>0;n--){
    if(*p1!=*p2)return *(unsigned char *)p1-*(unsigned char *)p2;
    else if(*p1=='\0')return 0;
    p1++;p2++;
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  //panic("Not implemented");
  unsigned char uc = (unsigned char)c;
  unsigned char *su;
  for (su = (unsigned char *)s; 0 < n; ++su, --n)
    *su = uc;
  return s;
}


void *memmove(void *dst, const void *src, size_t n) {
  //panic("Not implemented");
  if (dst < src){
    for (size_t i = 0; i < n; i++){
      *((char*)dst + i) = *((char*)src + i);
    }
  }
  else if (dst > src){
    for (int i = n-1; i>-1; i--){
      *((char*)dst + i) = *((char*)src + i);
    }
  }
  return dst;
}


void *memcpy(void *out, const void *in, size_t n) {
  //panic("Not implemented");
  return memmove(out, in, n);
}

int memcmp(const void *s1, const void *s2, size_t n) {
  //panic("Not implemented");
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    while (n--)
    {
        if (*p1 != *p2)
            return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

#endif
