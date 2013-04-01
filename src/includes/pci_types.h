/*
 * =======================================================================================
 *
 *      Filename:  pci_types.h
 *
 *      Description:  Types file for pci module.
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


#ifndef PCI_TYPES_H
#define PCI_TYPES_H

#include <stdint.h>

typedef enum {
    PCI_R3QPI_DEVICE_LINK_0 = 0,
    PCI_R3QPI_DEVICE_LINK_1,
    PCI_R2PCIE_DEVICE,
    PCI_IMC_DEVICE_CH_0,
    PCI_IMC_DEVICE_CH_1,
    PCI_IMC_DEVICE_CH_2,
    PCI_IMC_DEVICE_CH_3,
    PCI_HA_DEVICE,
    PCI_QPI_DEVICE_PORT_0,
    PCI_QPI_DEVICE_PORT_1,
    PCI_QPI_MASK_DEVICE_PORT_0,
    PCI_QPI_MASK_DEVICE_PORT_1,
    PCI_QPI_MISC_DEVICE_PORT_0,
    PCI_QPI_MISC_DEVICE_PORT_1,
    MAX_NUM_DEVICES
} PciDeviceIndex;

#endif /*PCI_TYPES_H*/
