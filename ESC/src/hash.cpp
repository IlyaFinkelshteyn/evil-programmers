/*
  [ESC] Editor's settings changer plugin for FAR Manager
  Copyright (C) 2000 Konstantin Stupnik
  Copyright (C) 2008 Alex Yaroslavsky

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  This unit used internally by xmlite.
  Special hashtable implementation.
  This table can hold several values with one key.
  Multihash.

*/

#include <CRT/crt.hpp>
#include <windows.h>
#include "xmem.h"
#include "hash.h"

static unsigned hHashFunc(const wchar_t *Key)
{
  wchar_t * curr = (wchar_t *)Key;
  long count = *curr;
  while(*curr)
  {
    count += 37 * count + *curr;
    curr++;
  }
  return ( (unsigned)( ( ( count * 19L ) + 12451L ) % 8882693L ) );
}

static unsigned hHashFuncEx(const wchar_t *Key,int keylen)
{
  wchar_t * curr = (wchar_t *)Key;
  long count = *curr;
  while(keylen--)
  {
    count += 37 * count + *curr;
    curr++;
  }
  return ( (unsigned)( ( ( count * 19L ) + 12451L ) % 8882693L ) );
}


static PHashLink hlnkNew(void* Pool,const wchar_t *Key,const void* Value)
{
  PHashLink p=(PHashLink)xalloc(Pool,sizeof(SHashLink));

#ifdef HASH_FLAG_ALLOCKEY
  p->szKey=xstrdup(Pool,Key);
#else
  p->szKey=Key;
#endif
  p->pValue=(void*)Value;
  p->uHashCode=hHashFunc(Key);
  p->pNext=NULL;
  return p;
}

static void hlnkFree(PHashLink p)
{
#ifdef HASH_FLAG_ALLOCKEY
  xfree(p->szKey);
#endif
  xfree(p);
}

/*
static PHashList hlNew(void* Pool)
{
  PHashList p;
  p=xcalloc(Pool,sizeof(SHashList));
  return p;
}

static void hlFree(PHashList l)
{
  PHashLink h,t;
  h=l->pHead;
  while(h)
  {
    t=h->pNext;
    hlnkFree(h);
    h=t;
  }
//  hfree(l);
}
*/

static PHashLink hlAdd(void* Pool,PHashList l,const wchar_t *Key,const void* Value)
{
  if(l->pHead)
  {
    l->pTail->pNext=hlnkNew(Pool,Key,Value);
    l->pTail=l->pTail->pNext;
  }else
  {
    l->pHead=hlnkNew(Pool,Key,Value);
    l->pTail=l->pHead;
  }
  return l->pTail;
}

static PHashLink hlAppend(PHashList l,PHashLink ln)
{
  ln->pNext=NULL;
  if(l->pHead)
  {
    l->pTail->pNext=ln;
    l->pTail=ln;
  }else
  {
    l->pHead=ln;
    l->pTail=ln;
  }
  return ln;
}

static PHashLink hlFind(PHashList l,const wchar_t *Key,int keylen)
{
  PHashLink tmp=l->pHead;
  while(tmp)
  {
    if(xstrncmp(tmp->szKey,Key,keylen))return tmp;
    tmp=tmp->pNext;
  }
  return NULL;
}

static PHashLink hlFindN(PHashList l,const wchar_t *Key,int N)
{
  PHashLink tmp=l->pHead;
  while(tmp)
  {
    if(xstrcmp(tmp->szKey,Key) && !N--)return tmp;
    tmp=tmp->pNext;
  }
  return NULL;
}

static int hlRemove(PHashList l,const wchar_t *Key)
{
  PHashLink tmp=l->pHead,last=NULL;
  while(tmp)
  {
    if(xstrcmp(tmp->szKey,Key))
    {
      if(last)
      {
        last->pNext=tmp->pNext;
        if(l->pTail==tmp)l->pTail=last;
        hlnkFree(tmp);
      }else
      {
        l->pHead=l->pHead->pNext;
        if(l->pTail==tmp)l->pTail=l->pHead;
        hlnkFree(tmp);
      }
      return 1;
    }
    last=tmp;
    tmp=tmp->pNext;
  }
  return 0;
}

static PHashLink hFindLink(PHash h,const wchar_t *Key)
{
  return hlFind(&h->pBuckets[hHashFunc(Key) % h->iBucketsNum],Key,lstrlen(Key));
}

static PHashLink hFindLinkEx(PHash h,const wchar_t *Key,int keylen,unsigned *Index)
{
  *Index=hHashFuncEx(Key,keylen) % h->iBucketsNum;
  return hlFind(&h->pBuckets[*Index],Key,keylen);
}

static PHashLink hFindLinkN(PHash h,const wchar_t *Key,int N)
{
  return hlFindN(&h->pBuckets[hHashFunc(Key) % h->iBucketsNum],Key,N);
}


PHash hashNew(void* Pool)
{
  return hashNewEx(Pool,HASH_DEFAULT_SIZE);
}

PHash hashNewEx(void* Pool,int cnt)
{
  PHash h=(PHash)xcalloc(Pool,sizeof(SHash));
  h->iBucketsNum=cnt;
  h->pBuckets=(PHashList)xcalloc(Pool,sizeof(SHashList)*h->iBucketsNum);
  h->iCount=0;
  h->pPool=Pool;
  return h;
}


void hashFree(PHash h)
{
/*  int i;
  if(h==NULL)return;
  for(i=0;i<h->iBucketsNum;i++)hlFree(&h->pBuckets[i]);
  xfree(h->pBuckets);
  xfree(h);*/
};



int hashExists(PHash h,const wchar_t *Key)
{
  if(!h)return 0;
  return hFindLink(h,Key)!=NULL;
}

void hashDelete(PHash h,const wchar_t *Key)
{
  unsigned Index;
  if(!h)return;
  Index=hHashFunc(Key) % h->iBucketsNum;
  while(hlRemove(&h->pBuckets[Index],Key))h->iCount--;
}

void *hashGet(PHash h,const wchar_t *Key)
{
  PHashLink link;
  if(!h)return NULL;
  link=hFindLink(h,Key);
  return link?link->pValue:NULL;
}

void* hashGetEx(PHash h,const wchar_t *Key,int keylen)
{
  PHashLink link;
  unsigned index;
  if(!h)return NULL;
  link=hFindLinkEx(h,Key,keylen,&index);
  return link?link->pValue:NULL;
}

void* hashGetN(PHash h,const wchar_t *Key,int N)
{
  PHashLink link;
  if(!h)return NULL;
  link=hFindLinkN(h,Key,N);
  return link?link->pValue:NULL;
}

PHashLink hashEnumKey(PHash h,const wchar_t *Key,PHashLink lnk,void**Value)
{
  if(!h)return NULL;
  if(lnk==(PHashLink)0xffffffff)return NULL;
  if(lnk)
  {
    if(xstrcmp(lnk->szKey,Key))
    {
      *Value=lnk->pValue;
      return lnk->pNext?lnk->pNext:(PHashLink)0xffffffff;
    }
    return NULL;
  }else
  {
    lnk=hFindLink(h,Key);
    if(lnk)
    {
      *Value=lnk->pValue;
      return lnk->pNext?lnk->pNext:(PHashLink)0xffffffff;
    }else return NULL;
  }
}

int hashKeyCount(PHash h,const wchar_t *Key)
{
  PHashLink link=hFindLink(h,Key);
  int Cnt=link?1:0;
  if(!h)return 0;
  while(link)
  {
    if(xstrcmp(link->szKey,Key))Cnt++;
    link=link->pNext;
  }
  return Cnt;
}

static void hashResize(PHash h)
{
  PHashList nb;
  PHashLink l1,l2;
  int n,i;
  n=h->iCount*2;
  nb=(PHashList)xcalloc(h->pPool,n*sizeof(SHashList));
  for(i=0;i<h->iBucketsNum;i++)
  {
    l1=h->pBuckets[i].pHead;
    while(l1)
    {
      l2=l1->pNext;
      hlAppend(&nb[l1->uHashCode% n],l1);
      l1=l2;
    }
  }
  xfree(h->pBuckets);
  h->pBuckets=nb;
  h->iBucketsNum=n;
}

PHashLink hashSet(PHash h,const wchar_t *Key,const void* Value)
{
  unsigned Index;
  PHashLink link=hFindLinkEx(h,Key,lstrlen(Key),&Index);
  if(link)
  {
    link->pValue=(void*)Value;
    return link;
  }else
  {
    h->iCount++;
    link=hlAdd(h->pPool,&h->pBuckets[Index],Key,Value);
    if(h->iCount>=h->iBucketsNum*2)
    {
      hashResize(h);
    }
    return link;
  }
}

PHashLink hashAdd(PHash h,const wchar_t *Key,void* Value)
{
  unsigned Index=hHashFunc(Key) % h->iBucketsNum;
  PHashLink link;
  h->iCount++;
  link=hlAdd(h->pPool,&h->pBuckets[Index],Key,Value);
  if(h->iCount>h->iBucketsNum*4)
  {
    hashResize(h);
  }
  return link;
}

void hashFirst(PHash h)
{
  h->iIterIndex=0;
  h->pIterLink=NULL;
};

int hashNext(PHash h,wchar_t ** pKey,void** pValue)
{
  if(h->iCount==0)return 0;
  if(h->iIterIndex>=h->iBucketsNum)return 0;
  if(!h->pIterLink)
  {
    while((h->pIterLink=h->pBuckets[h->iIterIndex].pHead)==NULL)
    {
      h->iIterIndex++;
      if(h->iIterIndex>=h->iBucketsNum)return 0;
    }
  }
  *pKey=h->pIterLink->szKey;
  *pValue=h->pIterLink->pValue;
  h->pIterLink=h->pIterLink->pNext;
  if(!h->pIterLink)h->iIterIndex++;
  return 1;
}
