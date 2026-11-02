#include "strbuf.h"
#include <ctype.h>
#include <errno.h>
void strbuf_init(struct strbuf *sb, size_t alloc)
{
    sb->len=0;
    sb->alloc=0;
    sb->buf=NULL;
    if(alloc>0)
    {
        sb->buf=malloc(alloc);
        if(sb->buf==NULL)
        {
            fprintf(stderr,"strbuf_init: malloc failed: %s\n",strerror(errno));
            exit(1);
        }
        sb->alloc=alloc;
        sb->buf[0]='\0';
    } 
}
void strbuf_attach(struct strbuf *sb, void *str, size_t len, size_t alloc)
{
    sb->buf=str;
    sb->len=len;
    sb->alloc=alloc;
}
void strbuf_release(struct strbuf *sb)
{
    if(sb->alloc)
    {
        free(sb->buf);
        sb->buf=NULL;
        sb->len=0;
        sb->alloc=0;
    }
}
void strbuf_swap(struct strbuf *a, struct strbuf *b)
{
    struct strbuf tmp=*a;
    *a=*b;
    *b=tmp;
}
char *strbuf_detach(struct strbuf *sb, size_t *sz)
{
    char *ret=sb->buf;

    if(sz!=NULL)
        *sz=sb->alloc;     
    sb->buf=NULL;  
    sb->len=0;
    sb->alloc=0;
    return ret;
}
int strbuf_cmp(const struct strbuf *first, const struct strbuf *second)
{
    size_t len=first->len>second->len?second->len:first->len;
    int cmp=memcmp(first->buf,second->buf,len);
    if(cmp!=0)
    {
        return cmp;
    }
    return first->len<second->len?-1:first->len>second->len?1:0;
}
void strbuf_grow(struct strbuf *sb, size_t extra)
{
    size_t need=(size_t)sb->len+extra+1;
    if((size_t)sb->alloc>=need)
    { 
        return;
    }
    char*q=realloc(sb->buf,need);
    if(q==NULL)
    {
        fprintf(stderr,"strbuf_grow: realloc failed: %s\n",strerror(errno));
        exit(1);
    }
    sb->buf=q;
    sb->alloc=need;
}
void strbuf_add(struct strbuf *sb, const void *data, size_t len)
{
    strbuf_grow(sb,len);
    memcpy(sb->buf+sb->len,data,len);
    sb->len+=len;
    sb->buf[sb->len]='\0';
}
void strbuf_addch(struct strbuf *sb, int c)
{
    strbuf_grow(sb,1);
    sb->buf[sb->len++]=(char)c;
    sb->buf[sb->len]='\0';
}
void strbuf_addstr(struct strbuf *sb, const char *s)
{
    strbuf_add(sb,s,strlen(s));
}
void strbuf_addbuf(struct strbuf *sb, const struct strbuf *sb2)
{
    strbuf_grow(sb,sb2->len);
    memcpy(sb->buf+sb->len,sb2->buf,sb2->len);
    sb->len+=sb2->len;
    sb->buf[sb->len]='\0';
}