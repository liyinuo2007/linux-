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
    int cmp=0;
    if(len>0)   /* 两个都空时 buf 可能是 NULL，memcmp(NULL,NULL,0) 属未定义行为 */
        cmp=memcmp(first->buf,second->buf,len);
    if(cmp!=0)
    {
        return cmp;
    }
    return first->len<second->len?-1:first->len>second->len?1:0;
}
void strbuf_reset(struct strbuf *sb)
{
    strbuf_setlen(sb,0);
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
    if(sb2->len==0)      /* 空 sb2 时 buf 可能是 NULL，memcpy(NULL,0) 属未定义行为 */
    return;
    strbuf_grow(sb,sb2->len);
    memcpy(sb->buf+sb->len,sb2->buf,sb2->len);
    sb->len+=sb2->len;
    sb->buf[sb->len]='\0';
}
void strbuf_setlen(struct strbuf *sb, size_t len)
{
    if(sb->buf==NULL&&len==0)
    {
        sb->len=0;
        return;
    }
    if((size_t)sb->len<len)
    {
        strbuf_grow(sb,len-sb->len);
    }
    sb->len=len;
    sb->buf[sb->len]='\0';
}
size_t strbuf_avail(const struct strbuf *sb)
{
    return sb->alloc?sb->alloc-sb->len-1:0;
}
void strbuf_insert(struct strbuf *sb, size_t pos, const void *data, size_t len)
{
    if(pos>(size_t)sb->len)
    pos=(size_t)sb->len;
    strbuf_grow(sb,len);
    memmove(sb->buf+pos+len,sb->buf+pos,(size_t)sb->len-pos);
    memcpy(sb->buf+pos,data,len);
    sb->len+=len;
    sb->buf[sb->len]='\0';
}
void strbuf_rtrim(struct strbuf *sb)
{
    while(sb->len>0&&isspace((unsigned char)sb->buf[sb->len-1]))
    sb->len--;
    if(sb->buf!=NULL)
    sb->buf[sb->len]='\0';
}
void strbuf_ltrim(struct strbuf *sb)
{
    size_t i=0;
    while(i<(size_t)sb->len&&isspace((unsigned char)sb->buf[i]))
    i++;
    if(i>0)
    {
        sb->len-=i;
        memmove(sb->buf,sb->buf+i,sb->len);
        sb->buf[sb->len]='\0';
    }
}
void strbuf_remove(struct strbuf *sb, size_t pos, size_t len)
{
    if(pos>(size_t)sb->len)
    return;
    if(len>sb->len-pos)
    len=(size_t)sb->len-pos;
    memmove(sb->buf+pos,sb->buf+len+pos,(size_t)sb->len-pos-len);
    sb->len-=len;
    sb->buf[sb->len]='\0';
}
ssize_t strbuf_read(struct strbuf *sb, int fd, size_t hint)
{
    size_t oldlen=sb->len;
    strbuf_grow(sb,hint?hint:8192);
    for(;;)
    {
        size_t space=(size_t)sb->alloc-(size_t)sb->len-1;
        ssize_t n=read(fd,sb->buf+sb->len,space);
        if(n<0)
        {
            if(errno==EINTR)
            continue;
            strbuf_setlen(sb, oldlen);
            return -1;
        }
        if(n==0)
        break;
        sb->len+=n;
        sb->buf[sb->len]='\0';
        if((size_t)sb->alloc-(size_t)sb->len-1==0)
        strbuf_grow(sb,8192);
    }
    return sb->len;
}
int strbuf_getline(struct strbuf *sb, FILE *fp)
{
    int ch;
    strbuf_reset(sb);
    while((ch=fgetc(fp))!=EOF&&ch!='\n')
    strbuf_addch(sb,ch);
    if(ch==EOF&&sb->len==0)
    return EOF;
    return 0;
}
struct strbuf **strbuf_split_buf(const char *str, size_t len, int terminator, int max)
{
    struct strbuf**ret=NULL;
    size_t nr=0;
    size_t cap=0;
    const char*p=str;
    size_t remain=len;
    while(remain>0)
    {
        size_t chunk=remain;
        if(max<=0||(int)nr+1<max)
        {
            const char *hit=memchr(p,terminator,remain);
            if(hit)
            {
                chunk=(size_t)(hit-p)+1;
            }
        }
    struct strbuf*t=malloc(sizeof(*t));
    if(t==NULL)
    exit(1);
    strbuf_init(t,chunk);
    strbuf_add(t,p,chunk);
    if(nr+2>cap)
    {
        size_t ncap=cap?cap*2:8;
        struct strbuf**q=realloc(ret,ncap*sizeof(*q));
        if(q==NULL)
        exit(1);
        ret=q;
        cap=ncap;
    }
    ret[nr++]=t;
    p+=chunk;
    remain-=chunk;
    }
    if(ret==NULL)
    {
        ret=malloc(sizeof(*ret));
        if(ret==NULL)
        exit(1);
    }
    ret[nr]=NULL;
    return ret;
}
bool strbuf_begin_judge(char *target_str, const char *str, int strnlen)
{
    if(target_str==NULL||str==NULL)
    return false;
    if(strnlen<=0)
    return true;
    return strncmp(target_str,str,(size_t)strnlen)==0;
}
char *strbuf_get_mid_buf(char *target_buf, int begin, int end, int len)
{
    if(target_buf==NULL||begin<0||end<begin||end>len)
    return NULL;
    int size=end-begin;
    char*mid_buf=malloc(size+1);
    if(mid_buf==NULL)
    return NULL;
    memcpy(mid_buf,target_buf+begin,size);
    mid_buf[size]='\0';
    return mid_buf;
}
/* 辅助函数：释放 strbuf_split_buf 返回的整个数组
 * （不在 strbuf.h 中，测试程序会自行声明后调用） */
void strbuf_list_free(struct strbuf **sbs)
{
    size_t i;

    if (sbs == NULL)
        return;
    for (i = 0; sbs[i] != NULL; i++) {
        strbuf_release(sbs[i]);
        free(sbs[i]);
    }
    free(sbs);
}
