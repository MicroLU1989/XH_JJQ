/*!
    \file    cdc_acm_core.h
    \brief   the header file of IAP driver

    \version 2023-06-30, V2.1.6, firmware for GD32F30x
*/

/*
    Copyright (c) 2025, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#ifndef DUAL_CDC_ACM_CORE_H
#define DUAL_CDC_ACM_CORE_H

#include "cdc_acm_core.h"

#define CDC_DATA_ITF_NUM             2U

#pragma pack(1)

typedef struct {
    usb_desc_header  header;              /*!< regular descriptor header containing the descriptor's type and length */
    uint8_t bFirstInterface;              /*!< bFirstInterface */
    uint8_t bInterfaceCount;              /*!< bInterfaceCount */
    uint8_t bFunctionClass;               /*!< bFunctionClass */
    uint8_t bFunctionSubClass;            /*!< bFunctionSubClass */
    uint8_t bFunctionProtocol;            /*!< bFunctionProtocol */
    uint8_t iFunction;                    /*!< iFunction  */
} usb_desc_IAD;

#pragma pack()

typedef struct {
    usb_desc_config                  config;                                /*!< configure descriptor */
    usb_desc_IAD                     cdc_iad0;                              /*!< IAD descriptor0 */
    usb_desc_itf                     cdc_loopback_interface0;               /*!< cdc cmd interface descriptor */
    usb_desc_header_func             cdc_loopback_header0;                  /*!< cdc header func descriptor */
    usb_desc_call_managment_func     cdc_loopback_call_managment0;          /*!< cdc call managment descriptor */
    usb_desc_acm_func                cdc_loopback_acm0;                     /*!< cdc acm descriptor */
    usb_desc_union_func              cdc_loopback_union0;                   /*!< cdc union func descriptor */
    usb_desc_ep                      cdc_loopback_cmd_endpoint0;            /*!< cdc cmd endpoint descriptor */
    usb_desc_itf                     cdc_loopback_data_interface0;          /*!< cdc data interface descriptor */
    usb_desc_ep                      cdc_loopback_out_endpoint0;            /*!< cdc out endpoint descriptor */
    usb_desc_ep                      cdc_loopback_in_endpoint0;             /*!< cdc in endpoint descriptor */
    usb_desc_IAD                     cdc_iad1;                              /*!< IAD descriptor1 */
    usb_desc_itf                     cdc_loopback_interface1;               /*!< cdc cmd interface descriptor */
    usb_desc_header_func             cdc_loopback_header1;                  /*!< cdc header func descriptor */
    usb_desc_call_managment_func     cdc_loopback_call_managment1;          /*!< cdc call managment descriptor */
    usb_desc_acm_func                cdc_loopback_acm1;                     /*!< cdc acm descriptor */
    usb_desc_union_func              cdc_loopback_union1;                   /*!< cdc union func descriptor */
    usb_desc_ep                      cdc_loopback_cmd_endpoint1;            /*!< cdc cmd endpoint descriptor */
    usb_desc_itf                     cdc_loopback_data_interface1;          /*!< cdc data interface descriptor */
    usb_desc_ep                      cdc_loopback_out_endpoint1;            /*!< cdc out endpoint descriptor */
    usb_desc_ep                      cdc_loopback_in_endpoint1;             /*!< cdc in endpoint descriptor */
} usb_dual_cdc_desc_config_set;

extern usb_desc dual_cdc_desc;
extern usb_class dual_cdc_class;

/* function declarations */
/* receive CDC ACM 0 data */
void cdc_data_receive0(usb_dev *udev);
/* receive CDC ACM 1 data */
void cdc_data_receive1(usb_dev *udev);
/* send CDC ACM 0 data */
void cdc_acm_data_send0(usb_dev *udev);
/* send CDC ACM 1 data */
void cdc_acm_data_send1(usb_dev *udev);
/* check CDC ACM 0 is ready for data transfer */
uint8_t cdc_check_ready0(usb_dev *udev);
/* check CDC ACM 1 is ready for data transfer */
uint8_t cdc_check_ready1(usb_dev *udev);

#endif /* DUAL_CDC_ACM_CORE_H */
