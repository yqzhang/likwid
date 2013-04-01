/*
 * =======================================================================================
 *
 *      Filename:  pci.h
 *
 *      Description:  Header File pci Module. 
 *
 *      Version:   3.0
 *      Released:  29.11.2012
 *
 *      Author:  Jan Treibig (jt), jan.treibig@gmail.com
 *      Project:  likwid
 *
 *      Copyright (c) 2012, Jan Treibig
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are met:
 *
 *      * Redistributions of source code must retain the above copyright notice, this
 *        list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above copyright notice,
 *        this list of conditions and the following disclaimer in the documentation
 *        and/or other materials provided with the distribution.
 *
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *      ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *      WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *      DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *      FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *      DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *      SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *      CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *      OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =======================================================================================
 */

#ifndef PCI_H
#define PCI_H

#include <types.h>


/* PCI config memory space access is addressed
 * BUS - DEVICE - FUNCTION
 * Listing for Uncore devices DEVICE.FUNCTION
 */

extern void pci_init();
extern void pci_finalize();
extern uint32_t pci_read(int cpu, PciDeviceIndex index, uint32_t reg);
extern void pci_write(int cpu, PciDeviceIndex index, uint32_t reg, uint32_t data);
extern uint32_t pci_tread(int socket_fd, int cpu, PciDeviceIndex index, uint32_t reg);
extern void pci_twrite(int socket_fd, int cpu, PciDeviceIndex index, uint32_t reg, uint32_t data);

#endif /* PCI_H */
