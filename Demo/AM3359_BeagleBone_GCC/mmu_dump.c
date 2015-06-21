/*
 * mmu_dump.c - Bare metal ARMv7 translation table dumper
 * Copyright 2014 Yifan Lu
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//extern int io_printf(UART0_BASE,const char *fmt, ...);
static int afe;
#include "cp15.h"
#include "am335.h"
#include "serial.h"

#define DESC_SEC        (0x2)
#define AP_RW           (3<<10) //supervisor=RW, user=RW
#define CB              (3<<2)  //cache_on, write_back
#define NCNB            (0<<2)  //cache_off,WR_BUF off
#define DOMAIN0         (0x0<<5)
#define RW_NCNB     (AP_RW|DOMAIN0|NCNB|DESC_SEC)   /* Read/Write without cache and write buffer */
#define RW_CB       (AP_RW|DOMAIN0|CB|DESC_SEC)     /* Read/Write, cache, write back */

static unsigned int pa2va(unsigned int pa)
{
    unsigned int va;
    unsigned int vaddr;
    unsigned int paddr;
    unsigned int i;

    va = 0;
    for (i = 0; i < 0x100000; i++)
    {
        vaddr =  i<< 12;
        /*physical address is translated to virtual address*/
        __asm__("mcr p15,0,%1,c7,c8,0\n\r\t"
                "mrc p15,0,%0,c7,c4,0\n\r\t" : "=r" (paddr) : "r" (vaddr));
        if ((pa & 0xFFFFF000) == (paddr & 0xFFFFF000))
        {
            va = vaddr + (pa & 0xFFF);
            break;
        }
    }
    return va;
}

static void mmu_get_perms(int ap2, int ap1, int *ur, int *uw, int *pr, int *pw)
{
    /* AFE enabled, use simple permissions */
    if (afe)
    {
        *ur = ap1 > 1;
        *uw = !ap2 && ap1 > 1;
        *pr = 1;
        *pw = !ap2 && ap1 < 2;
    }
    else
    {
        *pw = (!ap2 && ap1);
        *pr = *pw || ap1;
        *ur = ap1 > 1;
        *uw = !ap2 && ap1 == 3;
    }
}

static void mmu_dump_pages(unsigned int vaddr, unsigned int entry)
{
    int xn;
    int ng;
    int s;
    int ap2;
    int ap1;
    int pr;
    int pw;
    int ur;
    int uw;
    unsigned int paddr;

    if ((entry & 0x3) == 0x1) /* large page */
    {
        xn = entry & 0x8000;
        ng = entry & 0x800;
        s = entry & 0x400;
        ap2 = entry & 0x200;
        ap1 = (entry >> 4) & 3;
        mmu_get_perms(ap2, ap1, &ur, &uw, &pr, &pw);
        paddr = entry & 0xFFFF0000;
        io_printf(UART0_BASE,"-[0x%p] %s PA:0x%p NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n\r", vaddr, "Lg Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
    }
    else if ((entry & 0x2)) /* small page */
    {
        xn = entry & 1;
        ng = entry & 0x800;
        s = entry & 0x400;
        ap2 = entry & 0x200;
        ap1 = (entry >> 4) & 3;
        mmu_get_perms(ap2, ap1, &ur, &uw, &pr, &pw);
        paddr = entry & 0xFFFFF000;
        io_printf(UART0_BASE,"-[0x%p] %s PA:0x%p NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n\r", vaddr, "Sm Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
    }
    else
    {
        io_printf(UART0_BASE,"-[0x%p] %s\n\r", vaddr, "Unmapped ");
    }
}

static void mmu_dump_sections(unsigned int vaddr, unsigned int entry)
{
    int ns;
    int ss;
    int ng;
    int s;
    int ap1;
    int ap2;
    int domain;
    int xn;
    int pr;
    int pw;
    int ur;
    int uw;
    unsigned int paddr;
    unsigned int i;
    unsigned int test;
    unsigned int *tbl;
    unsigned int tblentry;

    if ((entry & 0x3) == 2) /* section or supersection */
    {
        ns = entry & 0x80000;
        ss = entry & 0x40000;
        ng = entry & 0x20000;
        s = entry & 0x10000;
        ap2 = entry & 0x8000;
        ap1 = (entry >> 10) & 3;
        domain = (entry >> 5) & 15;
        xn = entry & 0x10;
        mmu_get_perms(ap2, ap1, &ur, &uw, &pr, &pw);
        paddr = ss ? entry & 0xFF000000 : entry & 0xFFF00000;
        if(vaddr!=paddr)
            io_printf(UART0_BASE,"diff  ");
        io_printf(UART0_BASE,"entry=%p [0x%p] %s PA:0x%p NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d NS:%d DOM:%X\n\r\r",entry, vaddr, ss ? "S-Section " : "Section   ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn, !!ns, domain);
    }
    else if ((entry & 0x3) == 1) /* page table */
    {
        domain = (entry >> 5) & 15;
        ns = entry & 8;
        paddr = entry & 0xFFFFFC00;
        tbl = (unsigned int *)pa2va(paddr);
        io_printf(UART0_BASE,"entry=%p [0x%p] %s PA:0x%p VA:0x%p NS:%d DOM:%X\n\r",entry, vaddr, "Page TBL  ", paddr, tbl, !!ns, domain);
        for (i = 0; i < 0x100; i++)
        {
            mmu_dump_pages(vaddr+(i<<12), tbl[i]);
        }
    }
    else if ((entry & 0x3) == 0) /* not mapped */
    {
        io_printf(UART0_BASE,"entry=%p [0x%p] %s\n\r",entry, vaddr, "Unmapped  ");
    }
    else
    {
        io_printf(UART0_BASE,"entry=%p [0x%p] %s\n\r",entry, vaddr, "Invalid   ");
    }
}

int mmu_dump(void)
{
    unsigned int ttbr[2];
    int ttbcr;
    int n;
    unsigned int i;
    int paddr;
    unsigned int *ttb_vaddr[2];
    unsigned int entry;
    int contextid;
    __asm__("mrc p15,0,%0,c2,c0,0" : "=r" (ttbr[0]));//Translation Table Base Register 0
    __asm__("mrc p15,0,%0,c2,c0,1" : "=r" (ttbr[1]));//Translation Table Base Register 1
    __asm__("mrc p15,0,%0,c2,c0,2" : "=r" (ttbcr));//Translation Table Base Control Register
    io_printf(UART0_BASE,"TTBR0: 0x%p, TTBR1: 0x%p, TTBCR: 0x%p\n\r", ttbr[0], ttbr[1], ttbcr);

    n = ttbcr & 0x7;
    ttbr[0] &= (unsigned int)((int)0x80000000 >> (31 - 14 + 1 - n));
    ttbr[1] &= 0xFFFFC000;
    ttb_vaddr[0] = (unsigned int *)pa2va(ttbr[0]);
    ttb_vaddr[1] = (unsigned int *)pa2va(ttbr[1]);
    io_printf(UART0_BASE,"TBBR0 (physical): 0x%p, (virtual): 0x%p\n\r", ttbr[0], i<<12);
    io_printf(UART0_BASE,"TBBR1 (physical): 0x%p, (virtual): 0x%p\n\r", ttbr[1], i<<12);

    io_printf(UART0_BASE,"Dumping TTBR0...\n\r");
    for (i = 0; i < (1 << 12 - n); i++)
    {
        entry = ttb_vaddr[0][i];
        //io_printf(UART0_BASE,"  i=%d  addr_of_ttb_vaddr=%p ",i,&ttb_vaddr[0][i]);
        __asm__("DSB");
        __asm__("mrc p15, 0,%0,c13,c0,1":"=r"(contextid));
        if(contextid!=0)
        io_printf(UART0_BASE,"\r\n\r\n\r\ncontext id is %p\r\n", contextid);

        mmu_dump_sections(i<<20, entry);

    }


    if (n)
    {
        io_printf(UART0_BASE,"Dumping TTBR1...\n\r");
        for (i = ((~0xEFFF & 0xFFFF) >> n); i < 0x1000; i++)
        {
            entry = ttb_vaddr[1][i];
            mmu_dump_sections(i<<20, entry);
        }
    }
}

static volatile unsigned int _page_table[4*1024] __attribute__((aligned(16*1024)));
void mmu_setmtt(unsigned int vaddrStart, unsigned int vaddrEnd, unsigned int paddrStart, unsigned int attr)
{
    volatile unsigned int *pTT;
    volatile int i,nSec;
    pTT=(unsigned int *)_page_table+(vaddrStart>>20);
    nSec=(vaddrEnd>>20)-(vaddrStart>>20);
    for(i=0;i<=nSec;i++)
    {
        *pTT = attr |(((paddrStart>>20)+i)<<20);
        pTT++;
    }
}

int start_mmu(void)
{
    unsigned int sctlr;
    int result;

    CP15DCacheDisable();
    CP15ICacheDisable();
    CP15MMUDisable();
    CP15TlbInvalidate();
    CP15DomainAccessClientSet();
    /* set page table */
    mmu_setmtt(0x00000000, 0xFFFFFFFF, 0x00000000, RW_NCNB);    /* None cached for 4G memory    */
    mmu_setmtt(0x90000000, 0xB0000000-1, 0xA0600000, RW_CB);    /* 128M cached DDR memory       */
    mmu_setmtt(0xB0000000, 0xD8000000-1, 0xA0600000, RW_NCNB);  /* 128M none-cached DDR memory */
    mmu_setmtt(0x80000000, 0x80020000-1, 0x80000000, RW_CB);    /* 128k OnChip memory           */

    CP15Ttb0Set((unsigned int*) _page_table);
    CP15MMUEnable();
    CP15ICacheEnable();
    CP15DCacheEnable();

    __asm__("mrc p15,0,%0,c1,c0,0" : "=r" (sctlr));
    __asm__("orr %0,%1,#1 ":"=r"(result):"r"(sctlr));
    io_printf(UART0_BASE,"sctlr = %p \r\n",sctlr );
    afe = sctlr & 0x20000000;
    io_printf(UART0_BASE,"afe = %d\r\n",afe);

 //   mmu_dump();

    return 0;
}
