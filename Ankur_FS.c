F#define FUSE_USE_VERSION 26

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>

#define blocksize 1024

#define MB 1024*1024

int available_fd[50000]={0};
int m=0;
int r=0;

int getfd()
{
    int i=3;
    int mutex=0;

    while(mutex==1)
        continue;
    mutex=1;
    while(available_fd[i]==1)
        i++;
    available_fd[i]=1;
    mutex=0;
    return i;
}

void release_fd(int i)
{
    available_fd[i]=0;
}


int num=100;

long int max_size;
long int curr_size;
/*long int num_of_blocks;*/

struct directory{
char name[100];
int did;
int isfile;
struct directory *next_in_parent_dir;
struct directory *first_in_dir;
long int size;
char *data;
int isopen;
};
struct directory *root;

struct directory *lookup(const char *path)
{
    while(m==1)
    {
    }
    m=1;
    char *token,*tp;
    tp=malloc(sizeof(char));
    strcpy(tp,path);
    struct directory *ptr=root->first_in_dir;

    if(root->first_in_dir==NULL)
        {
            if(strcmp("/",tp)==0)
                {
                    m=0;return root;
                }
            else
                {
                    m=0;return NULL;
                }
        }

    else if(strcmp("/",tp)==0)
    {
                {
                    m=0;return root;
                }
    }

    token = strtok(tp,"/");
    while(token!=NULL)
    {
        while(ptr!=NULL){
            if(strcmp(ptr->name,token)==0)
                break;
            else
                ptr=ptr->next_in_parent_dir;
        }
        if(ptr==NULL)
        {
            m=0;return NULL;
        }
        token = strtok (NULL, "/");
        if(token!=NULL)
        {
            ptr=ptr->first_in_dir;
        }

    }
    m=0;
    return ptr;
}

static int ramdisk_getattr(const char *path, struct stat *stbuf)
{

	int res = 0;
	char *tp;
	tp=malloc(sizeof(char));
    strcpy(tp,path);

	struct directory *d=lookup(tp);

	while(m==1)
    {

    }
    m=1;

	if(d==NULL)
        {
            m=0;
            return -ENOENT;
        }

	memset(stbuf, 0, sizeof(struct stat));
	if (d->isfile==0) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
	} else if (d->isfile==1) {
		stbuf->st_mode = S_IFREG | 0766;
		stbuf->st_nlink = 1;
		stbuf->st_size = d->size;
	} else
		res = -ENOENT;

	m=0;
	return res;
}

int ramdisk_mkdir(const char *path, mode_t mode)
{

    int retstat = 0;
	char *token,*temp,*tp;
	temp=malloc(sizeof(char));
	tp=malloc(sizeof(char));
	strcpy(temp,path);
	strcpy(tp,path);
	temp=dirname(temp);
	tp=basename(tp);
    struct directory *d=lookup(temp);

    while(m==1)
    {

    }
    m=1;

    if(d->first_in_dir==NULL)
    {

        if((max_size-curr_size)>blocksize)
            curr_size=curr_size+blocksize;
        else
            {
                m=0;
                return -ENOMEM;
            }
        d->first_in_dir=malloc(blocksize);
        d->first_in_dir->did=++num;
        strcpy(d->first_in_dir->name,tp);
        fflush(stdout);
        d->first_in_dir->isfile=0;
        d->first_in_dir->next_in_parent_dir=NULL;
        d->first_in_dir->first_in_dir=NULL;
        d->first_in_dir->size=blocksize;
        d->first_in_dir->data=NULL;
        d->first_in_dir->isopen=0;
    }
    else
    {
        d=d->first_in_dir;
        while(d->next_in_parent_dir!=NULL)
            d=d->next_in_parent_dir;

        if((max_size-curr_size)>blocksize)
            curr_size=curr_size+blocksize;
        else
            {
                m=0;
                return -ENOMEM;
            }
        d->next_in_parent_dir=malloc(blocksize);
        d->next_in_parent_dir->did=++num;
        strcpy(d->next_in_parent_dir->name,tp);
        fflush(stdout);
        d->next_in_parent_dir->isfile=0;
        d->next_in_parent_dir->next_in_parent_dir=NULL;
        d->next_in_parent_dir->first_in_dir=NULL;
        d->next_in_parent_dir->size=blocksize;
        d->next_in_parent_dir->data=NULL;
        d->next_in_parent_dir->isopen=0;
    }

    mode=S_IFDIR | 0777;
    retstat=0;
    m=0;

    return retstat;
}

static int ramdisk_rmdir(const char *path)
{
    int res;
    long int len;
    char *token,*temp,*tp;
	tp=malloc(sizeof(char));
	temp=malloc(sizeof(char));
	strcpy(temp,path);
    strcpy(tp,path);
    temp=dirname(temp);
    struct directory *tempdir=lookup(temp);
    struct directory *d=lookup(tp);
    /*struct directory *ptr=root;*/
    /*struct directory *d=lookup(path);*/
    while(m==1)
    {

    }
    m=1;

    if(d==NULL)
    {
        m=0;
        return -ENOENT;
    }
    else if(d->first_in_dir!=NULL)
    {
        m=0;
        return -ENOTEMPTY;
    }

    if(strcmp(tempdir->first_in_dir->name,d->name)==0)
    {

            tempdir->first_in_dir=d->next_in_parent_dir;
            curr_size=curr_size-d->size;
            if(d->data==NULL)
              free(d);
            else{
                free(d->data);
                free(d);
            }
    }
    else
    {
        tempdir=tempdir->first_in_dir;
        while(strcmp(tempdir->next_in_parent_dir->name,d->name)!=0)
            tempdir=tempdir->next_in_parent_dir;
        tempdir->next_in_parent_dir=d->next_in_parent_dir;
        curr_size=curr_size-d->size;
        if(d->data==NULL)
          free(d);
        else{
          free(d->data);
          free(d);
        }
    }
        m=0;
        return 0;
}

int ramdisk_opendir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    char *tp;
    int fd;
	tp=malloc(sizeof(char));
    strcpy(tp,path);

	struct directory *d=lookup(tp);

	if(d==NULL)
        {
            return -ENOENT;
        }

    else if(d->isfile==1)
        {
            return -ENOTDIR;
        }

    fd=getfd();
    d->isopen++;
    fi->fh = fd;
    return retstat;
}

int ramdisk_releasedir(const char *path, struct fuse_file_info *fi)
{
        /*while(m==1)
        continue;*/

    int retstat = 0;
    char *tp;
	tp=malloc(sizeof(char));
    strcpy(tp,path);

	struct directory *d=lookup(tp);

	if(d==NULL)
        {
            return -ENOENT;
        }

    else if(d->isfile==1)
        {
            return -ENOTDIR;
        }


    release_fd(fi->fh);
    d->isopen--;
    return retstat;
}

static int ramdisk_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{

    (void) offset;
    (void) fi;
    int retstat = 0;
    char *tp;
	tp=malloc(sizeof(char));
    strcpy(tp,path);

	struct directory *d=lookup(tp);

	while(m==1)
    {

    }
    m=1;
    if (d==NULL)
    {
        m=0;
        return -ENOENT;
    }
    d=d->first_in_dir;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    while(d!=NULL)
    {
        filler(buf,d->name,NULL,0);
        d=d->next_in_parent_dir;
    }
    m=0;
    return 0;
}

int ramdisk_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{

    int retstat = 0;
    int fd;
    char *tp,*temp;
	tp=malloc(sizeof(char));
	temp=malloc(sizeof(char));
    strcpy(tp,path);
    strcpy(temp,path);
    temp=basename(temp);
    tp=dirname(tp);
	struct directory *d=lookup(tp);
	while(m==1)
    {

    }
    m=1;

	if(d==NULL)
        {
            m=0;
            return -ENOENT;
        }

    else if(d->isfile==1)
        {
            m=0;
            return -ENOTDIR;
        }

    if(d->first_in_dir==NULL)
    {
        d->first_in_dir=malloc(blocksize);
        d=d->first_in_dir;
    }
    else
    {
        d=d->first_in_dir;
        while(d->next_in_parent_dir!=NULL)
            d=d->next_in_parent_dir;
        d->next_in_parent_dir=malloc(blocksize);
        d=d->next_in_parent_dir;
    }
    if(max_size-curr_size>blocksize)
        curr_size=curr_size+blocksize;
    else
        {
            m=0;
            return -ENOMEM;
        }
    d->did=++num;
    strcpy(d->name,temp);
    d->isfile=1;
    d->next_in_parent_dir=NULL;
    d->first_in_dir=NULL;
    d->size=blocksize;
    d->data=NULL;
    d->isopen=1;

    fd = getfd();

    fi->fh = fd;

    mode = S_IFREG | 0666;
    m=0;
    return retstat;
}

int ramdisk_unlink(const char *path)
{

    int retstat = 0;
    char *token,*tp,*temp;
	tp=malloc(sizeof(char));
	temp=malloc(sizeof(char));
    strcpy(tp,path);
    strcpy(temp,path);
    temp=dirname(temp);
    /*struct directory *ptr=root->first_in_dir;*/
    struct directory *tempdir=lookup(temp);
	struct directory *d=lookup(tp);

	while(m==1)
    {

    }
    m=1;

	if(d==NULL)
        {
            m=0;
            return -ENOENT;
        }

    else if(d->isfile==0)
        {
            m=0;
            return -EISDIR;
        }

    /*else if(d->isopen>1)
        {
            m=0;
            return -EBUSY;
        }*/

    if(strcmp(tempdir->first_in_dir->name,d->name)==0)
    {

            tempdir->first_in_dir=d->next_in_parent_dir;
            curr_size=curr_size-d->size;
            if(d->data==NULL)
              free(d);
            else{
                free(d->data);
                free(d);
            }
    }
    else
    {
        tempdir=tempdir->first_in_dir;
        while(strcmp(tempdir->next_in_parent_dir->name,d->name)!=0)
            tempdir=tempdir->next_in_parent_dir;
        tempdir->next_in_parent_dir=d->next_in_parent_dir;
        curr_size=curr_size-d->size;
        if(d->data==NULL)
          free(d);
        else{
          free(d->data);
          free(d);
        }
    }

    m=0;
    return retstat;
}

int ramdisk_rename(const char *path, const char *newpath)
{
    int retstat=0;
    char *token,*tp,*temp;
	tp=malloc(sizeof(char));
	temp=malloc(sizeof(char));
    strcpy(tp,path);
    strcpy(temp,path);
    temp=dirname(temp);
    /*struct directory *ptr=root->first_in_dir;*/
    struct directory *tempdir=lookup(temp);
	struct directory *d=lookup(tp);

	char *token1,*tp1,*temp1;
	tp1=malloc(sizeof(char));
	temp1=malloc(sizeof(char));
    strcpy(tp1,newpath);
    strcpy(temp1,newpath);
    temp1=dirname(temp1);
    /*struct directory *ptr=root->first_in_dir;*/
    struct directory *d1=lookup(temp1);
	while(m==1)
    {

    }
    m=1;

	if(d==NULL)
        {
            m=0;
            return -ENOENT;
        }

    else if(d->isfile==0)
        {
            m=0;
            return -EISDIR;
        }
    if(d1==NULL)
        {
            m=0;
            return -ENOENT;
        }

    else if(d1->isfile==1)
        {
            m=0;
            return -ENOTDIR;
        }

    if(strcmp(tempdir->first_in_dir->name,d->name)==0)
    {

            tempdir->first_in_dir=d->next_in_parent_dir;
            curr_size=curr_size-d->size;
            if(d->data==NULL)
              free(d);
            else{
                free(d->data);
                free(d);
            }
    }
    else
    {
        tempdir=tempdir->first_in_dir;
        while(strcmp(tempdir->next_in_parent_dir->name,d->name)!=0)
            tempdir=tempdir->next_in_parent_dir;
        tempdir->next_in_parent_dir=d->next_in_parent_dir;
        curr_size=curr_size-d->size;
        if(d->data==NULL)
          free(d);
        else{
          free(d->data);
          free(d);
        }
    }


	if(d1->first_in_dir==NULL)
    {
        d1->first_in_dir=malloc(blocksize);
        d1=d1->first_in_dir;
    }
    else
    {
        d1=d1->first_in_dir;
        while(d1->next_in_parent_dir!=NULL)
            d1=d1->next_in_parent_dir;
        d1->next_in_parent_dir=malloc(blocksize);
        d1=d1->next_in_parent_dir;
    }

    d1->did=++num;
    strcpy(d1->name,temp1);
    d1->isfile=1;
    d1->next_in_parent_dir=NULL;
    d1->first_in_dir=NULL;
    d1->size=blocksize;
    d1->data=NULL;
    d1->isopen=1;

    m=0;


    return retstat;
}

int ramdisk_truncate(const char *path, off_t newsize)
{

    int retstat = 0;
    int fd;
    char *tp,temp;
	tp=malloc(sizeof(char));
    strcpy(tp,path);

    struct directory *d=lookup(tp);

    while(m==1)
    {

    }
    m=1;

    if(d==NULL)
        {
            m=0;
            return -ENOENT;
        }

    else if(d->isfile==0)
        {
            m=0;
            return -EISDIR;
        }
    if(newsize>d->size-blocksize)
    {
        d->data=realloc(d->data,newsize);
        curr_size=curr_size+newsize-d->size;
        d->size=newsize;
    }

    else if(newsize<d->size-blocksize)
    {
        d->data=realloc(d->data,newsize);
        curr_size=curr_size-(d->size-newsize);
        d->size=newsize;
    }
    else
    {

    }
    m=0;
    return retstat;
}

int ramdisk_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    int fd;
    char *tp;
	tp=malloc(sizeof(char));
    strcpy(tp,path);

	struct directory *d=lookup(tp);

	if(d==NULL)
        return -ENOENT;

    else if(d->isfile==0)
        return -EISDIR;

    fd = getfd();
    d->isopen++;
    fi->fh = fd;
    return retstat;
}

int ramdisk_release(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    char *tp;
	tp=malloc(sizeof(char));
    strcpy(tp,path);
    struct directory *d=lookup(tp);

    release_fd(fi->fh);
    d->isopen--;

    return retstat;
}

static int ramdisk_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
        size_t len;
        (void) fi;
        /*int i=0,j=0;*/
        char *tp,*temp;
        tp=malloc(sizeof(char));
        strcpy(tp,path);

        struct directory *d=lookup(tp);
        /*struct data_block *db=malloc(sizeof(data_block));
        db=d->data;*/
        while(r==1)
            {
            }
        while(m==1)
        {

        }
        m=1;

        if(d==NULL)
            {
                m=0;
                return -ENOENT;
            }
        else if(d->data==NULL)
            {
                m=0;
                return 0;
            }

        len=d->size-blocksize;
        if (offset < len) {
                if (offset + size > len)
                        size = len - offset;
                temp=malloc(size);
                strncpy(temp,d->data+offset,size);
                memcpy(buf,temp,size);
        } else
                size = 0;
        m=0;
        return size;
}

static int ramdisk_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{

        int fd;
        int res;
        (void) fi;
        int i=0,j=0,k=0;
        char *tp,*temp,*mem_temp;
        tp=malloc(sizeof(char));

        strcpy(tp,path);

        struct directory *d=lookup(tp);
        /*struct data_block *db=malloc(sizeof(data_block));
        db=d->data;*/
        while(m==1)
            {
            }
        m=1;
        r=1;
        if(d==NULL)
            {
                m=0;
                r=0;
                return -ENOENT;
            }
        else if(curr_size+size>max_size)
            {
                m=0;
                r=0;
                return -ENOMEM;
            }
        if(d->data==NULL)
        {
            d->data=malloc(size);
            d->size=d->size+size;
        }
        else{
        d->data=realloc(d->data,d->size+size+offset);
        d->size=d->size+size;
        curr_size=curr_size+size;
        }
        memcpy(d->data+offset,buf,size);
        m=0;
        r=0;
        return size;
}

static struct fuse_operations ramdisk = {
	.getattr	= ramdisk_getattr,
	.mkdir		= ramdisk_mkdir,
	.rmdir      = ramdisk_rmdir,
	.opendir    = ramdisk_opendir,
	.releasedir = ramdisk_releasedir,
	.readdir    = ramdisk_readdir,
	.create     = ramdisk_create,
	.unlink     = ramdisk_unlink,
	.rename     = ramdisk_rename,
	.truncate   = ramdisk_truncate,
	.open       = ramdisk_open,
	.release    = ramdisk_release,
	.read       = ramdisk_read,
	.write      = ramdisk_write
	};


int main(int argc, char *argv[])
{
	if(argc!=3)
    {
        printf("Usage:ramdisk <mount_point> <size>\n");
        exit(0);
    }
    max_size=atol(argv[2])*MB;

    root=malloc(blocksize);
    curr_size=blocksize;
    if (root == NULL) {
	perror("error in malloc");
	abort();
    }

    strcpy(root->name,"");
    root->did=num;
    root->isfile=0;
    root->next_in_parent_dir=NULL;
    root->first_in_dir=NULL;
    root->size=blocksize;
    root->data=NULL;
    root->isopen=0;
	return fuse_main(argc-1, argv, &ramdisk, root);
}
