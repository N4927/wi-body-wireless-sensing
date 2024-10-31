/* Automatically generated file; DO NOT EDIT */
/* Espressif IoT Development Framework Linker Script */
/* Generated from: C:\Espressif\Example_Projects\iPerf\iperf\build\esp-idf\esp_system\ld\sections.ld.in */

/*

 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD

 *

 * SPDX-License-Identifier: Apache-2.0

 */
/*

 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD

 *

 * SPDX-License-Identifier: Apache-2.0

 */
/*

 * Automatically generated file. DO NOT EDIT.

 * Espressif IoT Development Framework (ESP-IDF) 5.3.1 Configuration Header

 */
       
/* List of deprecated options */
/* CPU instruction prefetch padding size for flash mmap scenario */
/*

 * PMP region granularity size

 * Software may determine the PMP granularity by writing zero to pmp0cfg, then writing all ones

 * to pmpaddr0, then reading back pmpaddr0. If G is the index of the least-significant bit set,

 * the PMP granularity is 2^G+2 bytes.

 */
/* CPU instruction prefetch padding size for memory protection scenario */
/* Memory alignment size for PMS */
    /* rtc timer data (s_rtc_timer_retain_mem, see esp_clk.c files). For rtc_timer_data_in_rtc_mem section. */
/* Default entry point */
ENTRY(call_start_cpu0);
SECTIONS
{
  /**

   * RTC fast memory holds RTC wake stub code,

   * including from any source file named rtc_wake_stub*.c

   */
  .rtc.text :
  {
    
 . = ALIGN(4); 
 _rtc_fast_start = ABSOLUTE(.);
    
 . = ALIGN(4); 
 _rtc_text_start = ABSOLUTE(.);
    *(.rtc.entry.text)
    *(.rtc.literal .rtc.text .rtc.text.*)
    *rtc_wake_stub*.*(.text .text.*)
    *(.rtc_text_end_test)
    
 . = ALIGN(4); 
 _rtc_text_end = ABSOLUTE(.);
  } > lp_ram_seg
  /**

   * This section located in RTC FAST Memory area.

   * It holds data marked with RTC_FAST_ATTR attribute.

   * See the file "esp_attr.h" for more information.

   */
  .rtc.force_fast :
  {
    
 . = ALIGN(4); 
 _rtc_force_fast_start = ABSOLUTE(.);
    _coredump_rtc_fast_start = ABSOLUTE(.);
    *(.rtc.fast.coredump .rtc.fast.coredump.*)
    _coredump_rtc_fast_end = ABSOLUTE(.);
    *(.rtc.force_fast .rtc.force_fast.*)
    
 . = ALIGN(4); 
 _rtc_force_fast_end = ABSOLUTE(.);
  } > lp_ram_seg
  /**

   * RTC data section holds RTC wake stub

   * data/rodata, including from any source file

   * named rtc_wake_stub*.c and the data marked with

   * RTC_DATA_ATTR, RTC_RODATA_ATTR attributes.

   */
  .rtc.data :
  {
    _rtc_data_start = ABSOLUTE(.);
    _coredump_rtc_start = ABSOLUTE(.);
    *(.rtc.coredump .rtc.coredump.*)
    _coredump_rtc_end = ABSOLUTE(.);
    *(.rtc.data .rtc.data.*)
    *(.rtc.rodata .rtc.rodata.*)
    *rtc_wake_stub*.*(.data .rodata .data.* .rodata.* .srodata.*)
    _rtc_data_end = ABSOLUTE(.);
  } > lp_ram_seg
  /* RTC bss, from any source file named rtc_wake_stub*.c */
  .rtc.bss (NOLOAD) :
  {
    _rtc_bss_start = ABSOLUTE(.);
    *rtc_wake_stub*.*(.bss .bss.* .sbss .sbss.*)
    *rtc_wake_stub*.*(COMMON)
    *(.rtc.bss)
    _rtc_bss_end = ABSOLUTE(.);
  } > lp_ram_seg
  /**

   * This section holds data that should not be initialized at power up

   * and will be retained during deep sleep.

   * User data marked with RTC_NOINIT_ATTR will be placed

   * into this section. See the file "esp_attr.h" for more information.

   */
  .rtc_noinit (NOLOAD):
  {
    
 . = ALIGN(4); 
 _rtc_noinit_start = ABSOLUTE(.);
    *(.rtc_noinit .rtc_noinit.*)
    
 . = ALIGN(4); 
 _rtc_noinit_end = ABSOLUTE(.);
  } > lp_ram_seg
  /**

   * This section located in RTC SLOW Memory area.

   * It holds data marked with RTC_SLOW_ATTR attribute.

   * See the file "esp_attr.h" for more information.

   */
  .rtc.force_slow :
  {
    
 . = ALIGN(4); 
 _rtc_force_slow_start = ABSOLUTE(.);
    *(.rtc.force_slow .rtc.force_slow.*)
    
 . = ALIGN(4); 
 _rtc_force_slow_end = ABSOLUTE(.);
  } > lp_ram_seg
  /**

   * This section holds RTC data that should have fixed addresses.

   * The data are not initialized at power-up and are retained during deep

   * sleep.

   */
  .rtc_reserved (NOLOAD):
  {
    
 . = ALIGN(4); 
 _rtc_reserved_start = ABSOLUTE(.);
    /**

     * New data can only be added here to ensure existing data are not moved.

     * Because data have adhered to the end of the segment and code is relied

     * on it.

     * >> put new data here <<

     */
    *(.rtc_timer_data_in_rtc_mem .rtc_timer_data_in_rtc_mem.*)
    KEEP(*(.bootloader_data_rtc_mem .bootloader_data_rtc_mem.*))
    _rtc_reserved_end = ABSOLUTE(.);
  } > rtc_reserved_seg
  _rtc_reserved_length = _rtc_reserved_end - _rtc_reserved_start;
  ASSERT((_rtc_reserved_length <= LENGTH(rtc_reserved_seg)),
          "RTC reserved segment data does not fit.")
  /* Get size of rtc slow data based on rtc_data_location alias */
  _rtc_slow_length = (ORIGIN(rtc_slow_seg) == ORIGIN(rtc_data_location))
                        ? (_rtc_force_slow_end - _rtc_data_start)
                        : (_rtc_force_slow_end - _rtc_force_slow_start);
  _rtc_fast_length = (ORIGIN(rtc_slow_seg) == ORIGIN(rtc_data_location))
                        ? (_rtc_force_fast_end - _rtc_fast_start)
                        : (_rtc_noinit_end - _rtc_fast_start);
  ASSERT((_rtc_slow_length <= LENGTH(rtc_slow_seg)),
          "RTC_SLOW segment data does not fit.")
  ASSERT((_rtc_fast_length <= LENGTH(rtc_data_seg)),
          "RTC_FAST segment data does not fit.")
  .iram0.text :
  {
    _iram_start = ABSOLUTE(.);
    /* Vectors go to start of IRAM */
    ASSERT(ABSOLUTE(.) % 0x100 == 0, "vector address must be 256 byte aligned");
    KEEP(*(.exception_vectors_table.text));
    KEEP(*(.exception_vectors.text));
    
 . = ALIGN(4); 
 _invalid_pc_placeholder = ABSOLUTE(.);
    /* Code marked as running out of IRAM */
    _iram_text_start = ABSOLUTE(.);
    *(.iram1 .iram1.*)
    *libapp_trace.a:app_trace.*(.literal .literal.* .text .text.*)
    *libapp_trace.a:app_trace_util.*(.literal .literal.* .text .text.*)
    *libapp_trace.a:port_uart.*(.literal .literal.* .text .text.*)
    *libclang_rt.builtins.a:_divsf3.*(.literal .literal.* .text .text.*)
    *libclang_rt.builtins.a:restore.*(.literal .literal.* .text .text.*)
    *libclang_rt.builtins.a:save.*(.literal .literal.* .text .text.*)
    *libcoexist.a:(.coexiram .coexiram.*)
    *libcoexist.a:(.coexsleepiram .coexsleepiram.*)
    *libesp_driver_gptimer.a:gptimer.*(.literal.gptimer_default_isr .text.gptimer_default_isr)
    *libesp_event.a:default_event_loop.*(.literal.esp_event_isr_post .text.esp_event_isr_post)
    *libesp_event.a:esp_event.*(.literal.esp_event_isr_post_to .text.esp_event_isr_post_to)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_compare_and_set .text.esp_cpu_compare_and_set)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_reset .text.esp_cpu_reset)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_stall .text.esp_cpu_stall)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_unstall .text.esp_cpu_unstall)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_wait_for_intr .text.esp_cpu_wait_for_intr)
    *libesp_hw_support.a:esp_memory_utils.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:gdma.*(.literal.gdma_append .text.gdma_append)
    *libesp_hw_support.a:gdma.*(.literal.gdma_default_rx_isr .text.gdma_default_rx_isr)
    *libesp_hw_support.a:gdma.*(.literal.gdma_default_tx_isr .text.gdma_default_tx_isr)
    *libesp_hw_support.a:gdma.*(.literal.gdma_reset .text.gdma_reset)
    *libesp_hw_support.a:gdma.*(.literal.gdma_start .text.gdma_start)
    *libesp_hw_support.a:gdma.*(.literal.gdma_stop .text.gdma_stop)
    *libesp_hw_support.a:mspi_timing_tuning.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_module_reset .text.periph_module_reset)
    *libesp_hw_support.a:periph_ctrl.*(.literal.wifi_module_disable .text.wifi_module_disable)
    *libesp_hw_support.a:periph_ctrl.*(.literal.wifi_module_enable .text.wifi_module_enable)
    *libesp_hw_support.a:pmu_sleep.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:rtc_clk.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:rtc_time.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:sar_periph_ctrl.*(.literal.sar_periph_ctrl_power_enable .text.sar_periph_ctrl_power_enable)
    *libesp_hw_support.a:sleep_console.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:systimer.*(.literal .literal.* .text .text.*)
    *libesp_mm.a:esp_cache.*(.literal .literal.* .text .text.*)
    *libesp_netif.a:esp_netif_lwip.*(.literal.esp_netif_free_rx_buffer .text.esp_netif_free_rx_buffer)
    *libesp_netif.a:esp_netif_lwip.*(.literal.esp_netif_receive .text.esp_netif_receive)
    *libesp_netif.a:esp_netif_lwip.*(.literal.esp_netif_transmit_wrap .text.esp_netif_transmit_wrap)
    *libesp_netif.a:esp_pbuf_ref.*(.literal.esp_pbuf_allocate .text.esp_pbuf_allocate)
    *libesp_netif.a:esp_pbuf_ref.*(.literal.esp_pbuf_free .text.esp_pbuf_free)
    *libesp_netif.a:ethernetif.*(.literal.ethernet_low_level_output .text.ethernet_low_level_output)
    *libesp_netif.a:ethernetif.*(.literal.ethernetif_input .text.ethernetif_input)
    *libesp_netif.a:wlanif.*(.literal.low_level_output .text.low_level_output)
    *libesp_netif.a:wlanif.*(.literal.wlanif_input .text.wlanif_input)
    *libesp_phy.a:phy_init.*(.literal.esp_phy_disable .text.esp_phy_disable)
    *libesp_phy.a:phy_init.*(.literal.esp_phy_enable .text.esp_phy_enable)
    *libesp_phy.a:phy_init.*(.literal.esp_wifi_bt_power_domain_off .text.esp_wifi_bt_power_domain_off)
    *libesp_ringbuf.a:(.literal .literal.* .text .text.*)
    *libesp_rom.a:esp_rom_spiflash.*(.literal .literal.* .text .text.*)
    *libesp_rom.a:esp_rom_systimer.*(.literal .literal.* .text .text.*)
    *libesp_rom.a:esp_rom_tlsf.*(.literal .literal.* .text .text.*)
    *libesp_rom.a:esp_rom_wdt.*(.literal .literal.* .text .text.*)
    *libesp_system.a:esp_err.*(.literal .literal.* .text .text.*)
    *libesp_system.a:esp_system_chip.*(.literal.esp_system_abort .text.esp_system_abort)
    *libesp_system.a:image_process.*(.literal .literal.* .text .text.*)
    *libesp_system.a:ubsan.*(.literal .literal.* .text .text.*)
    *libesp_wifi.a:esp_adapter.*(.literal.coex_pti_get_wrapper .text.coex_pti_get_wrapper)
    *libesp_wifi.a:esp_adapter.*(.literal.wifi_clock_disable_wrapper .text.wifi_clock_disable_wrapper)
    *libesp_wifi.a:esp_adapter.*(.literal.wifi_clock_enable_wrapper .text.wifi_clock_enable_wrapper)
    *libesp_wifi.a:wifi_netif.*(.literal.wifi_sta_receive .text.wifi_sta_receive)
    *libesp_wifi.a:wifi_netif.*(.literal.wifi_transmit_wrap .text.wifi_transmit_wrap)
    *libfreertos.a:(EXCLUDE_FILE(*libfreertos.a:app_startup.* *libfreertos.a:freertos_compatibility.* *libfreertos.a:idf_additions.* *libfreertos.a:tasks.*) .literal EXCLUDE_FILE(*libfreertos.a:app_startup.* *libfreertos.a:freertos_compatibility.* *libfreertos.a:idf_additions.* *libfreertos.a:tasks.*) .literal.* EXCLUDE_FILE(*libfreertos.a:app_startup.* *libfreertos.a:freertos_compatibility.* *libfreertos.a:idf_additions.* *libfreertos.a:tasks.*) .text EXCLUDE_FILE(*libfreertos.a:app_startup.* *libfreertos.a:freertos_compatibility.* *libfreertos.a:idf_additions.* *libfreertos.a:tasks.*) .text.*)
    *libfreertos.a:tasks.*(.text .text.__getreent .text.eTaskGetState .text.pcTaskGetName .text.prvAddCurrentTaskToDelayedList .text.prvAddNewTaskToReadyList .text.prvDeleteTCB .text.prvIdleTask .text.prvInitialiseNewTask.constprop.0 .text.prvSearchForNameWithinSingleList .text.prvTaskPriorityRaise .text.prvTaskPriorityRestore .text.pvTaskGetCurrentTCBForCore .text.pvTaskGetThreadLocalStoragePointer .text.pvTaskIncrementMutexHeldCount .text.pxTaskGetStackStart .text.ulTaskGenericNotifyTake .text.ulTaskGenericNotifyValueClear .text.uxTaskGetNumberOfTasks .text.uxTaskGetStackHighWaterMark .text.uxTaskGetStackHighWaterMark2 .text.uxTaskPriorityGet .text.uxTaskPriorityGetFromISR .text.uxTaskResetEventItemValue .text.vTaskDelay .text.vTaskDelete .text.vTaskEndScheduler .text.vTaskGenericNotifyGiveFromISR .text.vTaskGetSnapshot .text.vTaskInternalSetTimeOutState .text.vTaskMissedYield .text.vTaskPlaceOnEventList .text.vTaskPlaceOnEventListRestricted .text.vTaskPlaceOnUnorderedEventList .text.vTaskPriorityDisinheritAfterTimeout .text.vTaskPrioritySet .text.vTaskRemoveFromUnorderedEventList .text.vTaskResume .text.vTaskSetThreadLocalStoragePointer .text.vTaskSetThreadLocalStoragePointerAndDelCallback .text.vTaskSetTimeOutState .text.vTaskStartScheduler .text.vTaskSuspend .text.vTaskSuspendAll .text.vTaskSwitchContext .text.xTaskAbortDelay .text.xTaskCatchUpTicks .text.xTaskCheckForTimeOut .text.xTaskCreatePinnedToCore .text.xTaskCreateStaticPinnedToCore .text.xTaskDelayUntil .text.xTaskGenericNotify .text.xTaskGenericNotifyFromISR .text.xTaskGenericNotifyStateClear .text.xTaskGenericNotifyWait .text.xTaskGetCoreID .text.xTaskGetCurrentTaskHandle .text.xTaskGetCurrentTaskHandleForCore .text.xTaskGetHandle .text.xTaskGetIdleTaskHandle .text.xTaskGetIdleTaskHandleForCore .text.xTaskGetSchedulerState .text.xTaskGetStaticBuffers .text.xTaskGetTickCount .text.xTaskGetTickCountFromISR .text.xTaskIncrementTick .text.xTaskIncrementTick.part.0 .text.xTaskPriorityDisinherit .text.xTaskPriorityInherit .text.xTaskRemoveFromEventList .text.xTaskResumeAll .text.xTaskResumeFromISR .text.xTimerCreateTimerTask)
    *libgcc.a:_divsf3.*(.literal .literal.* .text .text.*)
    *libgcc.a:lib2funcs.*(.literal .literal.* .text .text.*)
    *libgcc.a:save-restore.*(.literal .literal.* .text .text.*)
    *libgcov.a:(.literal .literal.* .text .text.*)
    *libhal.a:cache_hal.*(.literal .literal.* .text .text.*)
    *libhal.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_append .text.gdma_ahb_hal_append)
    *libhal.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_clear_intr .text.gdma_ahb_hal_clear_intr)
    *libhal.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_get_eof_desc_addr .text.gdma_ahb_hal_get_eof_desc_addr)
    *libhal.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_read_intr_status .text.gdma_ahb_hal_read_intr_status)
    *libhal.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_reset .text.gdma_ahb_hal_reset)
    *libhal.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_start_with_desc .text.gdma_ahb_hal_start_with_desc)
    *libhal.a:gdma_hal_ahb_v1.*(.literal.gdma_ahb_hal_stop .text.gdma_ahb_hal_stop)
    *libhal.a:gdma_hal_top.*(.literal.gdma_hal_append .text.gdma_hal_append)
    *libhal.a:gdma_hal_top.*(.literal.gdma_hal_clear_intr .text.gdma_hal_clear_intr)
    *libhal.a:gdma_hal_top.*(.literal.gdma_hal_get_eof_desc_addr .text.gdma_hal_get_eof_desc_addr)
    *libhal.a:gdma_hal_top.*(.literal.gdma_hal_read_intr_status .text.gdma_hal_read_intr_status)
    *libhal.a:gdma_hal_top.*(.literal.gdma_hal_reset .text.gdma_hal_reset)
    *libhal.a:gdma_hal_top.*(.literal.gdma_hal_start_with_desc .text.gdma_hal_start_with_desc)
    *libhal.a:gdma_hal_top.*(.literal.gdma_hal_stop .text.gdma_hal_stop)
    *libhal.a:i2c_hal_iram.*(.literal .literal.* .text .text.*)
    *libhal.a:ledc_hal_iram.*(.literal .literal.* .text .text.*)
    *libhal.a:mmu_hal.*(.literal .literal.* .text .text.*)
    *libhal.a:pmu_hal.*(.literal .literal.* .text .text.*)
    *libhal.a:spi_flash_encrypt_hal_iram.*(.literal .literal.* .text .text.*)
    *libhal.a:spi_flash_hal_gpspi.*(.literal .literal.* .text .text.*)
    *libhal.a:spi_flash_hal_iram.*(.literal .literal.* .text .text.*)
    *libhal.a:spi_hal_iram.*(.literal .literal.* .text .text.*)
    *libhal.a:spi_slave_hal_iram.*(.literal .literal.* .text .text.*)
    *libhal.a:timer_hal.*(.literal.timer_hal_capture_and_get_counter_value .text.timer_hal_capture_and_get_counter_value)
    *libheap.a:multi_heap.*(.literal._multi_heap_lock .text._multi_heap_lock)
    *libheap.a:multi_heap.*(.literal._multi_heap_unlock .text._multi_heap_unlock)
    *libheap.a:multi_heap.*(.literal.multi_heap_aligned_alloc_offs .text.multi_heap_aligned_alloc_offs)
    *libheap.a:multi_heap.*(.literal.multi_heap_in_rom_init .text.multi_heap_in_rom_init)
    *libieee802154.a:esp_ieee802154_ack.*(.literal.ieee802154_ack_config_pending_bit .text.ieee802154_ack_config_pending_bit)
    *libieee802154.a:esp_ieee802154_dev.*(.literal.ieee802154_isr .text.ieee802154_isr)
    *libieee802154.a:esp_ieee802154_dev.*(.literal.ieee802154_rx_frame_info_update .text.ieee802154_rx_frame_info_update)
    *libieee802154.a:esp_ieee802154_frame.*(.literal.ieee802154_frame_get_security_field_len .text.ieee802154_frame_get_security_field_len)
    *libieee802154.a:esp_ieee802154_frame.*(.literal.ieee802154_frame_get_security_payload_offset .text.ieee802154_frame_get_security_payload_offset)
    *libieee802154.a:esp_ieee802154_frame.*(.literal.ieee802154_frame_get_src_addr .text.ieee802154_frame_get_src_addr)
    *libieee802154.a:esp_ieee802154_frame.*(.literal.ieee802154_frame_security_header_offset .text.ieee802154_frame_security_header_offset)
    *libieee802154.a:esp_ieee802154_frame.*(.literal.is_dst_panid_present .text.is_dst_panid_present)
    *libieee802154.a:esp_ieee802154_frame.*(.literal.is_src_panid_present .text.is_src_panid_present)
    *libieee802154.a:esp_ieee802154_pib.*(.literal.ieee802154_pib_get_pending_mode .text.ieee802154_pib_get_pending_mode)
    *libieee802154.a:esp_ieee802154_pib.*(.literal.ieee802154_pib_get_rx_when_idle .text.ieee802154_pib_get_rx_when_idle)
    *libieee802154.a:esp_ieee802154_sec.*(.literal.ieee802154_sec_update .text.ieee802154_sec_update)
    *libieee802154.a:esp_ieee802154_sec.*(.literal.ieee802154_transmit_security_config .text.ieee802154_transmit_security_config)
    *libieee802154.a:esp_ieee802154_timer.*(.literal.ieee802154_timer0_set_threshold .text.ieee802154_timer0_set_threshold)
    *libieee802154.a:esp_ieee802154_timer.*(.literal.ieee802154_timer0_start .text.ieee802154_timer0_start)
    *libieee802154.a:esp_ieee802154_timer.*(.literal.ieee802154_timer0_stop .text.ieee802154_timer0_stop)
    *libieee802154.a:esp_ieee802154_timer.*(.literal.ieee802154_timer1_set_threshold .text.ieee802154_timer1_set_threshold)
    *libieee802154.a:esp_ieee802154_timer.*(.literal.ieee802154_timer1_start .text.ieee802154_timer1_start)
    *libieee802154.a:esp_ieee802154_timer.*(.literal.ieee802154_timer1_stop .text.ieee802154_timer1_stop)
    *libieee802154.a:esp_ieee802154_util.*(.literal.ieee802154_etm_channel_clear .text.ieee802154_etm_channel_clear)
    *liblog.a:log.*(.literal.esp_log_write .text.esp_log_write)
    *liblog.a:log_freertos.*(.literal.esp_log_early_timestamp .text.esp_log_early_timestamp)
    *liblog.a:log_freertos.*(.literal.esp_log_impl_lock .text.esp_log_impl_lock)
    *liblog.a:log_freertos.*(.literal.esp_log_impl_lock_timeout .text.esp_log_impl_lock_timeout)
    *liblog.a:log_freertos.*(.literal.esp_log_impl_unlock .text.esp_log_impl_unlock)
    *liblog.a:log_freertos.*(.literal.esp_log_timestamp .text.esp_log_timestamp)
    *liblwip.a:api_lib.*(.literal.netconn_recv .text.netconn_recv)
    *liblwip.a:api_lib.*(.literal.netconn_recv_data .text.netconn_recv_data)
    *liblwip.a:api_lib.*(.literal.netconn_recv_data_tcp .text.netconn_recv_data_tcp)
    *liblwip.a:api_lib.*(.literal.netconn_recv_tcp_pbuf_flags .text.netconn_recv_tcp_pbuf_flags)
    *liblwip.a:api_lib.*(.literal.netconn_recv_udp_raw_netbuf_flags .text.netconn_recv_udp_raw_netbuf_flags)
    *liblwip.a:api_lib.*(.literal.netconn_send .text.netconn_send)
    *liblwip.a:api_lib.*(.literal.netconn_sendto .text.netconn_sendto)
    *liblwip.a:api_lib.*(.literal.netconn_tcp_recvd .text.netconn_tcp_recvd)
    *liblwip.a:api_lib.*(.literal.netconn_write_partly .text.netconn_write_partly)
    *liblwip.a:api_lib.*(.literal.netconn_write_vectors_partly .text.netconn_write_vectors_partly)
    *liblwip.a:api_msg.*(.literal.lwip_netconn_do_send .text.lwip_netconn_do_send)
    *liblwip.a:api_msg.*(.literal.lwip_netconn_do_write .text.lwip_netconn_do_write)
    *liblwip.a:api_msg.*(.literal.recv_udp .text.recv_udp)
    *liblwip.a:etharp.*(.literal.etharp_output .text.etharp_output)
    *liblwip.a:etharp.*(.literal.etharp_output_to_arp_index .text.etharp_output_to_arp_index)
    *liblwip.a:ethernet.*(.literal.ethernet_input .text.ethernet_input)
    *liblwip.a:ethernet.*(.literal.ethernet_output .text.ethernet_output)
    *liblwip.a:inet_chksum.*(.literal.inet_chksum .text.inet_chksum)
    *liblwip.a:inet_chksum.*(.literal.inet_chksum_pbuf .text.inet_chksum_pbuf)
    *liblwip.a:inet_chksum.*(.literal.inet_chksum_pseudo .text.inet_chksum_pseudo)
    *liblwip.a:inet_chksum.*(.literal.inet_cksum_pseudo_base .text.inet_cksum_pseudo_base)
    *liblwip.a:inet_chksum.*(.literal.ip_chksum_pseudo .text.ip_chksum_pseudo)
    *liblwip.a:inet_chksum.*(.literal.lwip_standard_chksum .text.lwip_standard_chksum)
    *liblwip.a:ip4.*(.literal.ip4_input .text.ip4_input)
    *liblwip.a:ip4.*(.literal.ip4_output .text.ip4_output)
    *liblwip.a:ip4.*(.literal.ip4_output_if .text.ip4_output_if)
    *liblwip.a:ip4.*(.literal.ip4_output_if_opt .text.ip4_output_if_opt)
    *liblwip.a:ip4.*(.literal.ip4_output_if_opt_src .text.ip4_output_if_opt_src)
    *liblwip.a:ip4.*(.literal.ip4_output_if_src .text.ip4_output_if_src)
    *liblwip.a:ip4.*(.literal.ip4_route .text.ip4_route)
    *liblwip.a:ip4.*(.literal.ip4_route_src .text.ip4_route_src)
    *liblwip.a:ip4_addr.*(.literal.ip4_addr_isbroadcast_u32 .text.ip4_addr_isbroadcast_u32)
    *liblwip.a:lwip_default_hooks.*(.literal.ip4_route_src_hook .text.ip4_route_src_hook)
    *liblwip.a:mem.*(.literal.mem_malloc .text.mem_malloc)
    *liblwip.a:memp.*(.literal.memp_malloc .text.memp_malloc)
    *liblwip.a:netbuf.*(.literal.netbuf_alloc .text.netbuf_alloc)
    *liblwip.a:netbuf.*(.literal.netbuf_free .text.netbuf_free)
    *liblwip.a:pbuf.*(.literal.pbuf_add_header .text.pbuf_add_header)
    *liblwip.a:pbuf.*(.literal.pbuf_alloc .text.pbuf_alloc)
    *liblwip.a:pbuf.*(.literal.pbuf_alloced_custom .text.pbuf_alloced_custom)
    *liblwip.a:pbuf.*(.literal.pbuf_clone .text.pbuf_clone)
    *liblwip.a:pbuf.*(.literal.pbuf_copy .text.pbuf_copy)
    *liblwip.a:pbuf.*(.literal.pbuf_copy_partial_pbuf .text.pbuf_copy_partial_pbuf)
    *liblwip.a:pbuf.*(.literal.pbuf_free .text.pbuf_free)
    *liblwip.a:pbuf.*(.literal.pbuf_header .text.pbuf_header)
    *liblwip.a:pbuf.*(.literal.pbuf_remove_header .text.pbuf_remove_header)
    *liblwip.a:raw.*(.literal.raw_input .text.raw_input)
    *liblwip.a:sockets.*(.literal.done_socket .text.done_socket)
    *liblwip.a:sockets.*(.literal.event_callback .text.event_callback)
    *liblwip.a:sockets.*(.literal.lwip_recv_tcp .text.lwip_recv_tcp)
    *liblwip.a:sockets.*(.literal.lwip_recvfrom .text.lwip_recvfrom)
    *liblwip.a:sockets.*(.literal.lwip_select .text.lwip_select)
    *liblwip.a:sockets.*(.literal.lwip_send .text.lwip_send)
    *liblwip.a:sockets.*(.literal.lwip_sendto .text.lwip_sendto)
    *liblwip.a:sockets.*(.literal.tryget_socket_unconn_locked .text.tryget_socket_unconn_locked)
    *liblwip.a:sys_arch.*(.literal.sys_arch_mbox_fetch .text.sys_arch_mbox_fetch)
    *liblwip.a:sys_arch.*(.literal.sys_arch_protect .text.sys_arch_protect)
    *liblwip.a:sys_arch.*(.literal.sys_arch_sem_wait .text.sys_arch_sem_wait)
    *liblwip.a:sys_arch.*(.literal.sys_arch_unprotect .text.sys_arch_unprotect)
    *liblwip.a:sys_arch.*(.literal.sys_mbox_post .text.sys_mbox_post)
    *liblwip.a:sys_arch.*(.literal.sys_mbox_trypost .text.sys_mbox_trypost)
    *liblwip.a:sys_arch.*(.literal.sys_mutex_lock .text.sys_mutex_lock)
    *liblwip.a:sys_arch.*(.literal.sys_mutex_unlock .text.sys_mutex_unlock)
    *liblwip.a:sys_arch.*(.literal.sys_sem_signal .text.sys_sem_signal)
    *liblwip.a:sys_arch.*(.literal.sys_thread_tcpip .text.sys_thread_tcpip)
    *liblwip.a:tcp.*(.literal.tcp_fasttmr .text.tcp_fasttmr)
    *liblwip.a:tcp.*(.literal.tcp_process_refused_data .text.tcp_process_refused_data)
    *liblwip.a:tcp.*(.literal.tcp_recv_null .text.tcp_recv_null)
    *liblwip.a:tcp.*(.literal.tcp_recved .text.tcp_recved)
    *liblwip.a:tcp.*(.literal.tcp_seg_copy .text.tcp_seg_copy)
    *liblwip.a:tcp.*(.literal.tcp_seg_free .text.tcp_seg_free)
    *liblwip.a:tcp.*(.literal.tcp_segs_free .text.tcp_segs_free)
    *liblwip.a:tcp.*(.literal.tcp_slowtmr .text.tcp_slowtmr)
    *liblwip.a:tcp.*(.literal.tcp_tmr .text.tcp_tmr)
    *liblwip.a:tcp.*(.literal.tcp_update_rcv_ann_wnd .text.tcp_update_rcv_ann_wnd)
    *liblwip.a:tcp_in.*(.literal.tcp_input .text.tcp_input)
    *liblwip.a:tcp_in.*(.literal.tcp_input_delayed_close .text.tcp_input_delayed_close)
    *liblwip.a:tcp_in.*(.literal.tcp_oos_insert_segment .text.tcp_oos_insert_segment)
    *liblwip.a:tcp_in.*(.literal.tcp_parseopt .text.tcp_parseopt)
    *liblwip.a:tcp_in.*(.literal.tcp_receive .text.tcp_receive)
    *liblwip.a:tcp_out.*(.literal.tcp_create_segment .text.tcp_create_segment)
    *liblwip.a:tcp_out.*(.literal.tcp_output_control_segment .text.tcp_output_control_segment)
    *liblwip.a:tcp_out.*(.literal.tcp_rexmit .text.tcp_rexmit)
    *liblwip.a:tcp_out.*(.literal.tcp_rexmit_fast .text.tcp_rexmit_fast)
    *liblwip.a:tcp_out.*(.literal.tcp_rexmit_rto_prepare .text.tcp_rexmit_rto_prepare)
    *liblwip.a:tcp_out.*(.literal.tcp_route .text.tcp_route)
    *liblwip.a:tcp_out.*(.literal.tcp_rst .text.tcp_rst)
    *liblwip.a:tcp_out.*(.literal.tcp_send_empty_ack .text.tcp_send_empty_ack)
    *liblwip.a:tcp_out.*(.literal.tcp_split_unsent_seg .text.tcp_split_unsent_seg)
    *liblwip.a:tcp_out.*(.literal.tcp_write .text.tcp_write)
    *liblwip.a:tcpip.*(.literal.tcpip_callback .text.tcpip_callback)
    *liblwip.a:tcpip.*(.literal.tcpip_inpkt .text.tcpip_inpkt)
    *liblwip.a:tcpip.*(.literal.tcpip_input .text.tcpip_input)
    *liblwip.a:tcpip.*(.literal.tcpip_send_msg_wait_sem .text.tcpip_send_msg_wait_sem)
    *liblwip.a:tcpip.*(.literal.tcpip_thread .text.tcpip_thread)
    *liblwip.a:tcpip.*(.literal.tcpip_try_callback .text.tcpip_try_callback)
    *liblwip.a:udp.*(.literal.udp_input .text.udp_input)
    *liblwip.a:udp.*(.literal.udp_input_local_match .text.udp_input_local_match)
    *liblwip.a:udp.*(.literal.udp_send .text.udp_send)
    *liblwip.a:udp.*(.literal.udp_sendto .text.udp_sendto)
    *liblwip.a:udp.*(.literal.udp_sendto_if .text.udp_sendto_if)
    *liblwip.a:udp.*(.literal.udp_sendto_if_src .text.udp_sendto_if_src)
    *libnet80211.a:(.wifi0iram .wifi0iram.*)
    *libnet80211.a:(.wifiextrairam .wifiextrairam.*)
    *libnet80211.a:(.wifirxiram .wifirxiram.*)
    *libnet80211.a:(.wifislpiram .wifislpiram.*)
    *libnet80211.a:(.wifislprxiram .wifislprxiram.*)
    *libnewlib.a:abort.*(.literal .literal.* .text .text.*)
    *libnewlib.a:assert.*(.literal .literal.* .text .text.*)
    *libnewlib.a:heap.*(.literal .literal.* .text .text.*)
    *libnewlib.a:stdatomic.*(.literal .literal.* .text .text.*)
    *libpp.a:(.wifi0iram .wifi0iram.*)
    *libpp.a:(.wifiextrairam .wifiextrairam.*)
    *libpp.a:(.wifiorslpiram .wifiorslpiram.*)
    *libpp.a:(.wifirxiram .wifirxiram.*)
    *libpp.a:(.wifislpiram .wifislpiram.*)
    *libpp.a:(.wifislprxiram .wifislprxiram.*)
    *libriscv.a:interrupt.*(.text .text._global_interrupt_handler .text.intr_handler_get_arg)
    *libriscv.a:vectors.*(.literal .literal.* .text .text.*)
    *librtc.a:(.literal .literal.* .text .text.*)
    *libsoc.a:lldesc.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:flash_brownout_hook.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:memspi_host_driver.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_boya.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_gd.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_generic.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_issi.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_mxic.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_th.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_winbond.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_wrap.*(.literal .literal.* .text .text.*)
  } > sram_seg
  /* Marks the end of IRAM code segment */
  .iram0.text_end (NOLOAD) :
  {
    
 . = ALIGN(4); 
 _iram_text_end = ABSOLUTE(.);
  } > sram_seg
  .iram0.data :
  {
    
 . = ALIGN(16); 
 _iram_data_start = ABSOLUTE(.);
    *(.iram.data .iram.data.*)
    _coredump_iram_start = ABSOLUTE(.);
    *(.iram2.coredump .iram2.coredump.*)
    _coredump_iram_end = ABSOLUTE(.);
    _iram_data_end = ABSOLUTE(.);
  } > sram_seg
  .iram0.bss (NOLOAD) :
  {
    
 . = ALIGN(16); 
 _iram_bss_start = ABSOLUTE(.);
    *(.iram.bss .iram.bss.*)
    _iram_bss_end = ABSOLUTE(.);
    
 . = ALIGN(16); 
 _iram_end = ABSOLUTE(.);
  } > sram_seg
  .dram0.data :
  {
    _data_start = ABSOLUTE(.);
    *(.gnu.linkonce.d.*)
    *(.data1)
    __global_pointer$ = . + 0x800;
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.gnu.linkonce.s2.*)
    *(.jcr)
    *(.data .data.*)
    *(.dram1 .dram1.*)
    _coredump_dram_start = ABSOLUTE(.);
    *(.dram2.coredump .dram2.coredump.*)
    _coredump_dram_end = ABSOLUTE(.);
    *libapp_trace.a:app_trace.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libapp_trace.a:app_trace_util.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libapp_trace.a:port_uart.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libclang_rt.builtins.a:_divsf3.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libclang_rt.builtins.a:restore.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libclang_rt.builtins.a:save.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:esp_memory_utils.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:mspi_timing_tuning.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:pmu_sleep.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:rtc_clk.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:sleep_console.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:systimer.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_mm.a:esp_cache.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_spiflash.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_systimer.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_tlsf.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_wdt.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_system.a:esp_err.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_system.a:image_process.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_system.a:ubsan.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libgcc.a:_divsf3.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libgcc.a:save-restore.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libgcov.a:(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:cache_hal.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:i2c_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:ledc_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:mmu_hal.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:pmu_hal.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:spi_flash_encrypt_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:spi_flash_hal_gpspi.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:spi_flash_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:spi_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:spi_slave_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libnewlib.a:abort.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libnewlib.a:assert.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libnewlib.a:heap.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libnewlib.a:stdatomic.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libphy.a:(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libsoc.a:lldesc.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:flash_brownout_hook.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:memspi_host_driver.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_boya.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_gd.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_generic.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_issi.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_mxic.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_th.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_winbond.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_wrap.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    _data_end = ABSOLUTE(.);
  } > sram_seg
  /**

   * This section holds data that should not be initialized at power up.

   * The section located in Internal SRAM memory region. The macro _NOINIT

   * can be used as attribute to place data into this section.

   * See the "esp_attr.h" file for more information.

   */
  .noinit (NOLOAD):
  {
    
 . = ALIGN(4); 
 _noinit_start = ABSOLUTE(.);
    *(.noinit .noinit.*)
    
 . = ALIGN(4); 
 _noinit_end = ABSOLUTE(.);
  } > sram_seg
  /* Shared RAM */
  .dram0.bss (NOLOAD) :
  {
    
 . = ALIGN(8); 
 _bss_start = ABSOLUTE(.);
    /**

     * ldgen places all bss-related data to mapping[dram0_bss]

     * (See components/esp_system/app.lf).

     */
    *(.bss .bss.*)
    *(.dynbss .dynsbss .gnu.linkonce.b .gnu.linkonce.b.* .gnu.linkonce.sb .gnu.linkonce.sb.* .gnu.linkonce.sb2 .gnu.linkonce.sb2.* .sbss .sbss.* .sbss2 .sbss2.* .scommon .share.mem)
    *(.ext_ram.bss .ext_ram.bss.*)
    *(COMMON)
    
 . = ALIGN(8); 
 _bss_end = ABSOLUTE(.);
  } > sram_seg
  .flash.text :
  {
    _stext = .;
    /**

     * Mark the start of flash.text.

     * This can be used by the MMU driver to maintain the virtual address.

     */
    _instruction_reserved_start = ABSOLUTE(.);
    _text_start = ABSOLUTE(.);
    *(EXCLUDE_FILE(*libesp_ringbuf.a *libfreertos.a *libgcov.a *librtc.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libclang_rt.builtins.a:_divsf3.* *libclang_rt.builtins.a:restore.* *libclang_rt.builtins.a:save.* *libesp_driver_gptimer.a:gptimer.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:gdma.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:pmu_sleep.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:sar_periph_ctrl.* *libesp_hw_support.a:sleep_console.* *libesp_hw_support.a:systimer.* *libesp_mm.a:esp_cache.* *libesp_netif.a:esp_netif_lwip.* *libesp_netif.a:esp_pbuf_ref.* *libesp_netif.a:ethernetif.* *libesp_netif.a:wlanif.* *libesp_phy.a:phy_init.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_systimer.* *libesp_rom.a:esp_rom_tlsf.* *libesp_rom.a:esp_rom_wdt.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libesp_wifi.a:esp_adapter.* *libesp_wifi.a:wifi_netif.* *libgcc.a:_divsf3.* *libgcc.a:lib2funcs.* *libgcc.a:save-restore.* *libhal.a:cache_hal.* *libhal.a:gdma_hal_ahb_v1.* *libhal.a:gdma_hal_top.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:pmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_gpspi.* *libhal.a:spi_flash_hal_iram.* *libhal.a:spi_hal_iram.* *libhal.a:spi_slave_hal_iram.* *libhal.a:timer_hal.* *libheap.a:multi_heap.* *libieee802154.a:esp_ieee802154_ack.* *libieee802154.a:esp_ieee802154_dev.* *libieee802154.a:esp_ieee802154_frame.* *libieee802154.a:esp_ieee802154_pib.* *libieee802154.a:esp_ieee802154_sec.* *libieee802154.a:esp_ieee802154_timer.* *libieee802154.a:esp_ieee802154_util.* *liblog.a:log.* *liblog.a:log_freertos.* *liblwip.a:api_lib.* *liblwip.a:api_msg.* *liblwip.a:etharp.* *liblwip.a:ethernet.* *liblwip.a:inet_chksum.* *liblwip.a:ip4.* *liblwip.a:ip4_addr.* *liblwip.a:lwip_default_hooks.* *liblwip.a:mem.* *liblwip.a:memp.* *liblwip.a:netbuf.* *liblwip.a:pbuf.* *liblwip.a:raw.* *liblwip.a:sockets.* *liblwip.a:sys_arch.* *liblwip.a:tcp.* *liblwip.a:tcp_in.* *liblwip.a:tcp_out.* *liblwip.a:tcpip.* *liblwip.a:udp.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libriscv.a:interrupt.* *libriscv.a:vectors.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .literal EXCLUDE_FILE(*libesp_ringbuf.a *libfreertos.a *libgcov.a *librtc.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libclang_rt.builtins.a:_divsf3.* *libclang_rt.builtins.a:restore.* *libclang_rt.builtins.a:save.* *libesp_driver_gptimer.a:gptimer.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:gdma.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:pmu_sleep.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:sar_periph_ctrl.* *libesp_hw_support.a:sleep_console.* *libesp_hw_support.a:systimer.* *libesp_mm.a:esp_cache.* *libesp_netif.a:esp_netif_lwip.* *libesp_netif.a:esp_pbuf_ref.* *libesp_netif.a:ethernetif.* *libesp_netif.a:wlanif.* *libesp_phy.a:phy_init.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_systimer.* *libesp_rom.a:esp_rom_tlsf.* *libesp_rom.a:esp_rom_wdt.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libesp_wifi.a:esp_adapter.* *libesp_wifi.a:wifi_netif.* *libgcc.a:_divsf3.* *libgcc.a:lib2funcs.* *libgcc.a:save-restore.* *libhal.a:cache_hal.* *libhal.a:gdma_hal_ahb_v1.* *libhal.a:gdma_hal_top.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:pmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_gpspi.* *libhal.a:spi_flash_hal_iram.* *libhal.a:spi_hal_iram.* *libhal.a:spi_slave_hal_iram.* *libhal.a:timer_hal.* *libheap.a:multi_heap.* *libieee802154.a:esp_ieee802154_ack.* *libieee802154.a:esp_ieee802154_dev.* *libieee802154.a:esp_ieee802154_frame.* *libieee802154.a:esp_ieee802154_pib.* *libieee802154.a:esp_ieee802154_sec.* *libieee802154.a:esp_ieee802154_timer.* *libieee802154.a:esp_ieee802154_util.* *liblog.a:log.* *liblog.a:log_freertos.* *liblwip.a:api_lib.* *liblwip.a:api_msg.* *liblwip.a:etharp.* *liblwip.a:ethernet.* *liblwip.a:inet_chksum.* *liblwip.a:ip4.* *liblwip.a:ip4_addr.* *liblwip.a:lwip_default_hooks.* *liblwip.a:mem.* *liblwip.a:memp.* *liblwip.a:netbuf.* *liblwip.a:pbuf.* *liblwip.a:raw.* *liblwip.a:sockets.* *liblwip.a:sys_arch.* *liblwip.a:tcp.* *liblwip.a:tcp_in.* *liblwip.a:tcp_out.* *liblwip.a:tcpip.* *liblwip.a:udp.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libriscv.a:interrupt.* *libriscv.a:vectors.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .literal.* EXCLUDE_FILE(*libesp_ringbuf.a *libfreertos.a *libgcov.a *librtc.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libclang_rt.builtins.a:_divsf3.* *libclang_rt.builtins.a:restore.* *libclang_rt.builtins.a:save.* *libesp_driver_gptimer.a:gptimer.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:gdma.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:pmu_sleep.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:sar_periph_ctrl.* *libesp_hw_support.a:sleep_console.* *libesp_hw_support.a:systimer.* *libesp_mm.a:esp_cache.* *libesp_netif.a:esp_netif_lwip.* *libesp_netif.a:esp_pbuf_ref.* *libesp_netif.a:ethernetif.* *libesp_netif.a:wlanif.* *libesp_phy.a:phy_init.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_systimer.* *libesp_rom.a:esp_rom_tlsf.* *libesp_rom.a:esp_rom_wdt.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libesp_wifi.a:esp_adapter.* *libesp_wifi.a:wifi_netif.* *libgcc.a:_divsf3.* *libgcc.a:lib2funcs.* *libgcc.a:save-restore.* *libhal.a:cache_hal.* *libhal.a:gdma_hal_ahb_v1.* *libhal.a:gdma_hal_top.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:pmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_gpspi.* *libhal.a:spi_flash_hal_iram.* *libhal.a:spi_hal_iram.* *libhal.a:spi_slave_hal_iram.* *libhal.a:timer_hal.* *libheap.a:multi_heap.* *libieee802154.a:esp_ieee802154_ack.* *libieee802154.a:esp_ieee802154_dev.* *libieee802154.a:esp_ieee802154_frame.* *libieee802154.a:esp_ieee802154_pib.* *libieee802154.a:esp_ieee802154_sec.* *libieee802154.a:esp_ieee802154_timer.* *libieee802154.a:esp_ieee802154_util.* *liblog.a:log.* *liblog.a:log_freertos.* *liblwip.a:api_lib.* *liblwip.a:api_msg.* *liblwip.a:etharp.* *liblwip.a:ethernet.* *liblwip.a:inet_chksum.* *liblwip.a:ip4.* *liblwip.a:ip4_addr.* *liblwip.a:lwip_default_hooks.* *liblwip.a:mem.* *liblwip.a:memp.* *liblwip.a:netbuf.* *liblwip.a:pbuf.* *liblwip.a:raw.* *liblwip.a:sockets.* *liblwip.a:sys_arch.* *liblwip.a:tcp.* *liblwip.a:tcp_in.* *liblwip.a:tcp_out.* *liblwip.a:tcpip.* *liblwip.a:udp.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libriscv.a:interrupt.* *libriscv.a:vectors.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .text EXCLUDE_FILE(*libesp_ringbuf.a *libfreertos.a *libgcov.a *librtc.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libclang_rt.builtins.a:_divsf3.* *libclang_rt.builtins.a:restore.* *libclang_rt.builtins.a:save.* *libesp_driver_gptimer.a:gptimer.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:gdma.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:pmu_sleep.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:sar_periph_ctrl.* *libesp_hw_support.a:sleep_console.* *libesp_hw_support.a:systimer.* *libesp_mm.a:esp_cache.* *libesp_netif.a:esp_netif_lwip.* *libesp_netif.a:esp_pbuf_ref.* *libesp_netif.a:ethernetif.* *libesp_netif.a:wlanif.* *libesp_phy.a:phy_init.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_systimer.* *libesp_rom.a:esp_rom_tlsf.* *libesp_rom.a:esp_rom_wdt.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libesp_wifi.a:esp_adapter.* *libesp_wifi.a:wifi_netif.* *libgcc.a:_divsf3.* *libgcc.a:lib2funcs.* *libgcc.a:save-restore.* *libhal.a:cache_hal.* *libhal.a:gdma_hal_ahb_v1.* *libhal.a:gdma_hal_top.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:pmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_gpspi.* *libhal.a:spi_flash_hal_iram.* *libhal.a:spi_hal_iram.* *libhal.a:spi_slave_hal_iram.* *libhal.a:timer_hal.* *libheap.a:multi_heap.* *libieee802154.a:esp_ieee802154_ack.* *libieee802154.a:esp_ieee802154_dev.* *libieee802154.a:esp_ieee802154_frame.* *libieee802154.a:esp_ieee802154_pib.* *libieee802154.a:esp_ieee802154_sec.* *libieee802154.a:esp_ieee802154_timer.* *libieee802154.a:esp_ieee802154_util.* *liblog.a:log.* *liblog.a:log_freertos.* *liblwip.a:api_lib.* *liblwip.a:api_msg.* *liblwip.a:etharp.* *liblwip.a:ethernet.* *liblwip.a:inet_chksum.* *liblwip.a:ip4.* *liblwip.a:ip4_addr.* *liblwip.a:lwip_default_hooks.* *liblwip.a:mem.* *liblwip.a:memp.* *liblwip.a:netbuf.* *liblwip.a:pbuf.* *liblwip.a:raw.* *liblwip.a:sockets.* *liblwip.a:sys_arch.* *liblwip.a:tcp.* *liblwip.a:tcp_in.* *liblwip.a:tcp_out.* *liblwip.a:tcpip.* *liblwip.a:udp.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libriscv.a:interrupt.* *libriscv.a:vectors.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .text.*)
    *(EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifi0iram EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifi0iram.*)
    *(EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifiextrairam EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifiextrairam.*)
    *(EXCLUDE_FILE(*libpp.a) .wifiorslpiram EXCLUDE_FILE(*libpp.a) .wifiorslpiram.*)
    *(EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifirxiram EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifirxiram.*)
    *(EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifislpiram EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifislpiram.*)
    *(EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifislprxiram EXCLUDE_FILE(*libnet80211.a *libpp.a) .wifislprxiram.*)
    *libesp_driver_gptimer.a:gptimer.*(.text .text.gptimer_del_timer .text.gptimer_destroy .text.gptimer_disable .text.gptimer_enable .text.gptimer_get_captured_count .text.gptimer_get_raw_count .text.gptimer_get_resolution .text.gptimer_new_timer .text.gptimer_register_event_callbacks .text.gptimer_release_group_handle .text.gptimer_set_alarm_action .text.gptimer_set_raw_count .text.gptimer_start .text.gptimer_stop)
    *libesp_event.a:default_event_loop.*(.text .text.esp_event_handler_instance_register .text.esp_event_handler_instance_unregister .text.esp_event_handler_register .text.esp_event_handler_unregister .text.esp_event_loop_create_default .text.esp_event_loop_delete_default .text.esp_event_post)
    *libesp_event.a:esp_event.*(.text .text.base_node_add_handler .text.esp_event_dump .text.esp_event_handler_instance_register_with .text.esp_event_handler_instance_unregister_with .text.esp_event_handler_register_with .text.esp_event_handler_register_with_internal .text.esp_event_handler_unregister_with .text.esp_event_handler_unregister_with_internal .text.esp_event_loop_create .text.esp_event_loop_delete .text.esp_event_loop_run .text.esp_event_loop_run_task .text.esp_event_post_to .text.handler_instances_add .text.handler_instances_remove .text.handler_instances_remove_all .text.loop_node_add_handler)
    *libesp_hw_support.a:cpu.*(.text .text.esp_cpu_clear_breakpoint .text.esp_cpu_clear_watchpoint .text.esp_cpu_set_breakpoint .text.esp_cpu_set_watchpoint)
    *libesp_hw_support.a:gdma.*(.text .text.gdma_apply_strategy .text.gdma_config_transfer .text.gdma_connect .text.gdma_del_channel .text.gdma_del_rx_channel .text.gdma_del_tx_channel .text.gdma_disconnect .text.gdma_get_alignment_constraints .text.gdma_get_channel_id .text.gdma_get_free_m2m_trig_id_mask .text.gdma_new_ahb_channel .text.gdma_register_rx_event_callbacks .text.gdma_register_tx_event_callbacks .text.gdma_release_group_handle .text.gdma_release_pair_handle .text.gdma_set_priority)
    *libesp_hw_support.a:periph_ctrl.*(.text .text.periph_module_disable .text.periph_module_enable .text.periph_rcc_acquire_enter .text.periph_rcc_acquire_exit .text.periph_rcc_release_enter .text.periph_rcc_release_exit)
    *libesp_hw_support.a:sar_periph_ctrl.*(.text .text.s_sar_power_acquire .text.s_sar_power_release .text.sar_periph_ctrl_adc_continuous_power_acquire .text.sar_periph_ctrl_adc_continuous_power_release .text.sar_periph_ctrl_adc_oneshot_power_acquire .text.sar_periph_ctrl_adc_oneshot_power_release .text.sar_periph_ctrl_init .text.sar_periph_ctrl_power_disable .text.sar_periph_ctrl_pwdet_power_acquire .text.sar_periph_ctrl_pwdet_power_release)
    *libesp_netif.a:esp_netif_lwip.*(.text .text.esp_ip4addr_aton .text.esp_ip4addr_ntoa .text.esp_netif_add_ip6_address .text.esp_netif_add_ip6_address_api .text.esp_netif_api_cb .text.esp_netif_attach .text.esp_netif_create_ip6_linklocal .text.esp_netif_create_ip6_linklocal_api .text.esp_netif_deinit .text.esp_netif_destroy .text.esp_netif_destroy_api .text.esp_netif_dhcpc_get_status .text.esp_netif_dhcpc_option .text.esp_netif_dhcpc_option_api .text.esp_netif_dhcpc_start .text.esp_netif_dhcpc_start_api .text.esp_netif_dhcpc_stop .text.esp_netif_dhcpc_stop_api .text.esp_netif_dhcps_cb .text.esp_netif_dhcps_get_clients_by_mac .text.esp_netif_dhcps_get_clients_by_mac_api .text.esp_netif_dhcps_get_status .text.esp_netif_dhcps_option .text.esp_netif_dhcps_option_api .text.esp_netif_dhcps_start .text.esp_netif_dhcps_start_api .text.esp_netif_dhcps_stop .text.esp_netif_dhcps_stop_api .text.esp_netif_down .text.esp_netif_down_api .text.esp_netif_find_if .text.esp_netif_find_if_api .text.esp_netif_get_all_ip6 .text.esp_netif_get_default_netif .text.esp_netif_get_desc .text.esp_netif_get_dns_info .text.esp_netif_get_dns_info_api .text.esp_netif_get_event_id .text.esp_netif_get_flags .text.esp_netif_get_handle_from_ifkey .text.esp_netif_get_handle_from_netif_impl .text.esp_netif_get_hostname .text.esp_netif_get_ifkey .text.esp_netif_get_io_driver .text.esp_netif_get_ip6_global .text.esp_netif_get_ip6_linklocal .text.esp_netif_get_ip_info .text.esp_netif_get_mac .text.esp_netif_get_netif_impl .text.esp_netif_get_netif_impl_index .text.esp_netif_get_netif_impl_name .text.esp_netif_get_netif_impl_name_api .text.esp_netif_get_old_ip_info .text.esp_netif_get_route_prio .text.esp_netif_init .text.esp_netif_ip6_get_addr_type .text.esp_netif_ip_lost_timer .text.esp_netif_is_netif_up .text.esp_netif_is_valid_static_ip .text.esp_netif_join_ip6_multicast_group .text.esp_netif_join_ip6_multicast_group_api .text.esp_netif_leave_ip6_multicast_group .text.esp_netif_leave_ip6_multicast_group_api .text.esp_netif_lwip_remove .text.esp_netif_napt_disable .text.esp_netif_napt_enable .text.esp_netif_netstack_buf_free .text.esp_netif_netstack_buf_ref .text.esp_netif_new .text.esp_netif_new_api .text.esp_netif_ppp_set_auth .text.esp_netif_ppp_set_auth_api .text.esp_netif_remove_ip6_address .text.esp_netif_remove_ip6_address_api .text.esp_netif_set_default_netif .text.esp_netif_set_dns_info .text.esp_netif_set_dns_info_api .text.esp_netif_set_driver_config .text.esp_netif_set_hostname .text.esp_netif_set_hostname_api .text.esp_netif_set_ip4_addr .text.esp_netif_set_ip_info .text.esp_netif_set_ip_info_api .text.esp_netif_set_ip_old_info_api .text.esp_netif_set_link_speed .text.esp_netif_set_mac .text.esp_netif_set_mac_api .text.esp_netif_set_old_ip_info .text.esp_netif_start .text.esp_netif_start_api .text.esp_netif_stop .text.esp_netif_stop_api .text.esp_netif_str_to_ip4 .text.esp_netif_str_to_ip6 .text.esp_netif_tcpip_exec .text.esp_netif_transmit .text.esp_netif_up .text.esp_netif_up_api .text.esp_netif_update_default_netif_lwip .text.get_handle_from_ifkey_api .text.netif_callback_fn .text.netif_send_garp .text.netif_send_mldv6 .text.tcpip_exec_api .text.tcpip_init_done)
    *libesp_netif.a:esp_pbuf_ref.*(.text)
    *libesp_netif.a:ethernetif.*(.text .text.ethernetif_init)
    *libesp_netif.a:wlanif.*(.text .text.wlanif_init .text.wlanif_init_ap .text.wlanif_init_nan .text.wlanif_init_sta)
    *libesp_phy.a:phy_init.*(.text .text.esp_phy_erase_cal_data_in_nvs .text.esp_phy_get_init_data .text.esp_phy_load_cal_and_init .text.esp_phy_load_cal_data_from_nvs .text.esp_phy_modem_deinit .text.esp_phy_modem_init .text.esp_phy_release_init_data .text.esp_phy_store_cal_data_to_nvs .text.esp_phy_update_country_info .text.phy_get_lock .text.store_cal_data_to_nvs_handle)
    *libesp_system.a:esp_system_chip.*(.text .text.esp_get_free_heap_size .text.esp_get_free_internal_heap_size .text.esp_get_idf_version .text.esp_get_minimum_free_heap_size)
    *libesp_wifi.a:esp_adapter.*(.text .text.clear_intr_wrapper .text.coex_deinit_wrapper .text.coex_disable_wrapper .text.coex_enable_wrapper .text.coex_init_wrapper .text.coex_register_start_cb_wrapper .text.coex_schm_curr_period_get_wrapper .text.coex_schm_curr_phase_get_wrapper .text.coex_schm_flexible_period_get_wrapper .text.coex_schm_flexible_period_set_wrapper .text.coex_schm_interval_get_wrapper .text.coex_schm_process_restart_wrapper .text.coex_schm_register_cb_wrapper .text.coex_schm_status_bit_clear_wrapper .text.coex_schm_status_bit_set_wrapper .text.coex_wifi_channel_set_wrapper .text.coex_wifi_request_wrapper .text.disable_intr_wrapper .text.enable_intr_wrapper .text.esp_event_post_wrapper .text.esp_log_write_wrapper .text.esp_log_writev_wrapper .text.esp_phy_disable_wrapper .text.esp_phy_enable_wrapper .text.esp_read_mac_wrapper .text.event_group_wait_bits_wrapper .text.get_time_wrapper .text.mutex_create_wrapper .text.mutex_delete_wrapper .text.nvs_open_wrapper .text.queue_create_wrapper .text.queue_recv_wrapper .text.queue_send_to_back_wrapper .text.queue_send_to_front_wrapper .text.queue_send_wrapper .text.recursive_mutex_create_wrapper .text.regdma_link_set_write_wait_content_wrapper .text.set_intr_wrapper .text.set_isr_wrapper .text.sleep_retention_find_link_by_id_wrapper .text.task_create_pinned_to_core_wrapper .text.task_create_wrapper .text.task_get_max_priority_wrapper .text.wifi_create_queue .text.wifi_create_queue_wrapper .text.wifi_delete_queue .text.wifi_delete_queue_wrapper .text.wifi_reset_mac_wrapper .text.wifi_thread_semphr_free .text.wifi_thread_semphr_get_wrapper)
    *libesp_wifi.a:wifi_netif.*(.text .text.esp_wifi_create_if_driver .text.esp_wifi_destroy_if_driver .text.esp_wifi_get_if_mac .text.esp_wifi_is_if_ready_when_started .text.esp_wifi_register_if_rxcb .text.wifi_ap_receive .text.wifi_driver_start .text.wifi_free .text.wifi_transmit)
    *libfreertos.a:app_startup.*(.literal .literal.* .text .text.*)
    *libfreertos.a:freertos_compatibility.*(.literal .literal.* .text .text.*)
    *libfreertos.a:idf_additions.*(.literal .literal.* .text .text.*)
    *libfreertos.a:tasks.*(.literal.uxTaskGetSnapshotAll .text.uxTaskGetSnapshotAll)
    *libfreertos.a:tasks.*(.literal.xTaskGetNext .text.xTaskGetNext)
    *libhal.a:gdma_hal_ahb_v1.*(.text .text.gdma_ahb_hal_connect_peri .text.gdma_ahb_hal_disconnect_peri .text.gdma_ahb_hal_enable_burst .text.gdma_ahb_hal_enable_etm_task .text.gdma_ahb_hal_enable_intr .text.gdma_ahb_hal_get_intr_status_reg .text.gdma_ahb_hal_init .text.gdma_ahb_hal_set_priority .text.gdma_ahb_hal_set_strategy)
    *libhal.a:gdma_hal_top.*(.text .text.gdma_hal_connect_peri .text.gdma_hal_deinit .text.gdma_hal_disconnect_peri .text.gdma_hal_enable_access_encrypt_mem .text.gdma_hal_enable_burst .text.gdma_hal_enable_etm_task .text.gdma_hal_enable_intr .text.gdma_hal_get_intr_status_reg .text.gdma_hal_set_burst_size .text.gdma_hal_set_priority .text.gdma_hal_set_strategy)
    *libhal.a:timer_hal.*(.text .text.timer_hal_deinit .text.timer_hal_init .text.timer_hal_set_counter_value)
    *libheap.a:multi_heap.*(.text .text.multi_heap_reset_minimum_free_bytes .text.multi_heap_restore_minimum_free_bytes)
    *libieee802154.a:esp_ieee802154_ack.*(.text .text.ieee802154_add_pending_addr .text.ieee802154_clear_pending_addr .text.ieee802154_reset_pending_table)
    *libieee802154.a:esp_ieee802154_dev.*(.text .text.ieee802154_cca .text.ieee802154_disable .text.ieee802154_enable .text.ieee802154_energy_detect .text.ieee802154_get_recent_lqi .text.ieee802154_get_recent_rssi .text.ieee802154_get_state .text.ieee802154_mac_deinit .text.ieee802154_mac_init .text.ieee802154_receive .text.ieee802154_receive_at .text.ieee802154_receive_handle_done .text.ieee802154_sleep .text.ieee802154_transmit .text.ieee802154_transmit_at .text.set_next_rx_buffer .text.stop_current_operation.isra.0 .text.stop_tx.isra.0)
    *libieee802154.a:esp_ieee802154_frame.*(.text .text.ieee802154_frame_get_dest_panid .text.ieee802154_frame_get_dst_addr .text.ieee802154_frame_get_src_panid .text.ieee802154_frame_get_type .text.ieee802154_frame_get_version .text.ieee802154_frame_is_ack_required)
    *libieee802154.a:esp_ieee802154_pib.*(.text .text.ieee802154_pib_get_auto_ack_rx .text.ieee802154_pib_get_auto_ack_tx .text.ieee802154_pib_get_cca_mode .text.ieee802154_pib_get_cca_threshold .text.ieee802154_pib_get_channel .text.ieee802154_pib_get_coordinator .text.ieee802154_pib_get_enhance_ack_tx .text.ieee802154_pib_get_power .text.ieee802154_pib_get_promiscuous .text.ieee802154_pib_init .text.ieee802154_pib_is_pending .text.ieee802154_pib_set_auto_ack_rx .text.ieee802154_pib_set_auto_ack_tx .text.ieee802154_pib_set_cca_mode .text.ieee802154_pib_set_cca_threshold .text.ieee802154_pib_set_channel .text.ieee802154_pib_set_coordinator .text.ieee802154_pib_set_enhance_ack_tx .text.ieee802154_pib_set_pending_mode .text.ieee802154_pib_set_power .text.ieee802154_pib_set_promiscuous .text.ieee802154_pib_set_rx_when_idle .text.ieee802154_pib_update)
    *libieee802154.a:esp_ieee802154_sec.*(.text)
    *libieee802154.a:esp_ieee802154_timer.*(.text .text.ieee802154_timer0_get_value .text.ieee802154_timer1_get_value)
    *libieee802154.a:esp_ieee802154_util.*(.text .text.ieee802154_channel_to_freq .text.ieee802154_etm_set_event_task .text.ieee802154_freq_to_channel .text.ieee802154_set_txrx_pti)
    *liblog.a:log.*(.text .text.esp_log_level_get .text.esp_log_level_set .text.esp_log_set_vprintf .text.esp_log_writev .text.heap_bubble_down .text.s_log_level_get_and_unlock)
    *liblog.a:log_freertos.*(.text .text.esp_log_system_timestamp)
    *liblwip.a:api_lib.*(.text .text.netconn_accept .text.netconn_bind .text.netconn_bind_if .text.netconn_close .text.netconn_connect .text.netconn_delete .text.netconn_disconnect .text.netconn_err .text.netconn_getaddr .text.netconn_gethostbyname_addrtype .text.netconn_join_leave_group .text.netconn_join_leave_group_netif .text.netconn_listen_with_backlog .text.netconn_new_with_proto_and_callback .text.netconn_prepare_delete .text.netconn_recv_tcp_pbuf .text.netconn_recv_udp_raw_netbuf .text.netconn_shutdown .text.netconn_thread_cleanup .text.netconn_thread_init)
    *liblwip.a:api_msg.*(.text .text.accept_function .text.err_tcp .text.lwip_netconn_do_accepted .text.lwip_netconn_do_bind .text.lwip_netconn_do_bind_if .text.lwip_netconn_do_close .text.lwip_netconn_do_close_internal .text.lwip_netconn_do_connect .text.lwip_netconn_do_connected .text.lwip_netconn_do_delconn .text.lwip_netconn_do_disconnect .text.lwip_netconn_do_dns_found .text.lwip_netconn_do_getaddr .text.lwip_netconn_do_gethostbyname .text.lwip_netconn_do_join_leave_group .text.lwip_netconn_do_join_leave_group_netif .text.lwip_netconn_do_listen .text.lwip_netconn_do_newconn .text.lwip_netconn_do_recv .text.lwip_netconn_do_writemore .text.lwip_netconn_is_deallocated_msg .text.lwip_netconn_is_err_msg .text.netconn_alloc .text.netconn_drain .text.netconn_free .text.netconn_mark_mbox_invalid .text.poll_tcp .text.recv_raw .text.recv_tcp .text.sent_tcp .text.setup_tcp .text.tcp_do_abort)
    *liblwip.a:etharp.*(.text .text.etharp_add_static_entry .text.etharp_cleanup_netif .text.etharp_find_addr .text.etharp_find_entry .text.etharp_free_entry .text.etharp_get_entry .text.etharp_input .text.etharp_query .text.etharp_raw .text.etharp_remove_static_entry .text.etharp_request .text.etharp_tmr .text.etharp_update_arp_entry)
    *liblwip.a:ethernet.*(.text)
    *liblwip.a:inet_chksum.*(.text .text.inet_chksum_pseudo_partial .text.inet_cksum_pseudo_partial_base .text.ip6_chksum_pseudo .text.ip6_chksum_pseudo_partial .text.ip_chksum_pseudo_partial)
    *liblwip.a:ip4.*(.text .text.ip4_set_default_multicast_netif)
    *liblwip.a:ip4_addr.*(.text .text.ip4_addr_netmask_valid .text.ip4addr_aton .text.ip4addr_ntoa .text.ip4addr_ntoa_r .text.ipaddr_addr)
    *liblwip.a:lwip_default_hooks.*(.text .text.dhcp_append_extra_opts .text.dhcp_parse_extra_opts .text.ip4_netif_exist)
    *liblwip.a:mem.*(.text .text.mem_calloc .text.mem_free .text.mem_init .text.mem_trim)
    *liblwip.a:memp.*(.text .text.do_memp_free_pool.part.0 .text.do_memp_malloc_pool.isra.0 .text.memp_free .text.memp_free_pool .text.memp_init .text.memp_init_pool .text.memp_malloc_pool)
    *liblwip.a:netbuf.*(.text .text.netbuf_chain .text.netbuf_data .text.netbuf_delete .text.netbuf_first .text.netbuf_new .text.netbuf_next .text.netbuf_ref)
    *liblwip.a:pbuf.*(.text .text.pbuf_add_header_force .text.pbuf_alloc_reference .text.pbuf_cat .text.pbuf_chain .text.pbuf_clen .text.pbuf_clone.part.0 .text.pbuf_coalesce .text.pbuf_copy_partial .text.pbuf_dechain .text.pbuf_free_header .text.pbuf_free_ooseq_callback .text.pbuf_get_at .text.pbuf_get_contiguous .text.pbuf_header_force .text.pbuf_memcmp .text.pbuf_memfind .text.pbuf_put_at .text.pbuf_realloc .text.pbuf_ref .text.pbuf_ref.part.0 .text.pbuf_skip .text.pbuf_strstr .text.pbuf_take .text.pbuf_take_at .text.pbuf_try_get_at)
    *liblwip.a:raw.*(.text .text.raw_bind .text.raw_bind_netif .text.raw_connect .text.raw_disconnect .text.raw_netif_ip_addr_changed .text.raw_new .text.raw_new_ip_type .text.raw_recv .text.raw_remove .text.raw_send .text.raw_sendto .text.raw_sendto_if_src)
    *liblwip.a:sockets.*(.text .text.alloc_socket .text.free_socket .text.lwip_accept .text.lwip_bind .text.lwip_close .text.lwip_connect .text.lwip_fcntl .text.lwip_getaddrname .text.lwip_getpeername .text.lwip_getsockname .text.lwip_getsockopt .text.lwip_inet_ntop .text.lwip_inet_pton .text.lwip_ioctl .text.lwip_listen .text.lwip_poll .text.lwip_pollscan .text.lwip_read .text.lwip_readv .text.lwip_recv .text.lwip_recvfrom_udp_raw.constprop.0 .text.lwip_recvmsg .text.lwip_select_inc_sockets_used_set.part.0 .text.lwip_selscan .text.lwip_sendmsg .text.lwip_setsockopt .text.lwip_shutdown .text.lwip_sock_make_addr.isra.0 .text.lwip_socket .text.lwip_socket_dbg_get_socket .text.lwip_socket_thread_cleanup .text.lwip_socket_thread_init .text.lwip_unlink_select_cb .text.lwip_write .text.lwip_writev .text.sockaddr_to_ipaddr_port .text.tryget_socket_unconn)
    *liblwip.a:sys_arch.*(.text .text.sys_arch_mbox_tryfetch .text.sys_delay_ms .text.sys_init .text.sys_jiffies .text.sys_mbox_free .text.sys_mbox_new .text.sys_mbox_set_owner .text.sys_mbox_trypost_fromisr .text.sys_mutex_free .text.sys_mutex_lock.part.0 .text.sys_mutex_new .text.sys_mutex_unlock.part.0 .text.sys_now .text.sys_sem_free .text.sys_sem_new .text.sys_sem_signal_isr .text.sys_thread_new .text.sys_thread_sem_deinit .text.sys_thread_sem_free .text.sys_thread_sem_get .text.sys_thread_sem_init)
    *liblwip.a:tcp.*(.text .text.tcp_abandon .text.tcp_abort .text.tcp_accept .text.tcp_accept_null .text.tcp_alloc .text.tcp_arg .text.tcp_backlog_accepted .text.tcp_backlog_delayed .text.tcp_bind .text.tcp_bind_netif .text.tcp_close .text.tcp_close_ext .text.tcp_close_shutdown .text.tcp_close_shutdown_fin .text.tcp_connect .text.tcp_debug_state_str .text.tcp_eff_send_mss_netif .text.tcp_err .text.tcp_free .text.tcp_free.part.0 .text.tcp_free_ooseq .text.tcp_init .text.tcp_kill_state .text.tcp_listen_with_backlog .text.tcp_listen_with_backlog_and_err .text.tcp_netif_ip_addr_changed .text.tcp_netif_ip_addr_changed_pcblist .text.tcp_new .text.tcp_new_ip_type .text.tcp_new_port .text.tcp_next_iss .text.tcp_pcb_purge .text.tcp_pcb_purge.part.0 .text.tcp_pcb_remove .text.tcp_poll .text.tcp_recv .text.tcp_sent .text.tcp_setprio .text.tcp_shutdown .text.tcp_tcp_get_tcp_addrinfo .text.tcp_txnow)
    *liblwip.a:tcp_in.*(.text .text.tcp_free_acked_segments.isra.0 .text.tcp_trigger_input_pcb_close)
    *liblwip.a:tcp_out.*(.text .text.tcp_enqueue_flags .text.tcp_keepalive .text.tcp_output .text.tcp_output_alloc_header.constprop.0 .text.tcp_output_alloc_header_common.constprop.0 .text.tcp_output_segment_busy.part.0 .text.tcp_rexmit_rto .text.tcp_rexmit_rto_commit .text.tcp_rexmit_rto_commit.part.0 .text.tcp_send_fin .text.tcp_zero_window_probe)
    *liblwip.a:tcpip.*(.text .text.mem_free_callback .text.pbuf_free_callback .text.pbuf_free_int .text.tcpip_api_call .text.tcpip_callbackmsg_delete .text.tcpip_callbackmsg_new .text.tcpip_callbackmsg_trycallback .text.tcpip_callbackmsg_trycallback_fromisr .text.tcpip_init)
    *liblwip.a:udp.*(.text .text.udp_bind .text.udp_bind_netif .text.udp_connect .text.udp_disconnect .text.udp_init .text.udp_netif_ip_addr_changed .text.udp_new .text.udp_new_ip_type .text.udp_recv .text.udp_remove)
    *libriscv.a:interrupt.*(.literal.intr_handler_get .text.intr_handler_get)
    *libriscv.a:interrupt.*(.literal.intr_handler_set .text.intr_handler_set)
    *(.stub)
    *(.gnu.linkonce.t.*)
    *(.gnu.warning)
    *(.irom0.text) /* catch stray ICACHE_RODATA_ATTR */
    /**

     * CPU will try to prefetch up to 16 bytes of of instructions.

     * This means that any configuration (e.g. MMU, PMS) must allow

     * safe access to up to 16 bytes after the last real instruction, add

     * dummy bytes to ensure this

     */
    . += 16;
    _text_end = ABSOLUTE(.);
    /**

     * Mark the flash.text end.

     * This can be used for MMU driver to maintain virtual address.

     */
    _instruction_reserved_end = ABSOLUTE(.);
    _etext = .;
    /**

     * Similar to _iram_start, this symbol goes here so it is

     * resolved by addr2line in preference to the first symbol in

     * the flash.text segment.

     */
    _flash_cache_start = ABSOLUTE(0);
  } > default_code_seg
  /**

   * Dummy section represents the .flash.text section but in default_rodata_seg.

   * Thus, it must have its alignment and (at least) its size.

   */
  .flash_rodata_dummy (NOLOAD):
  {
    _flash_rodata_dummy_start = .;
    . = ALIGN(ALIGNOF(.flash.text)) + SIZEOF(.flash.text);
    /* Add alignment of MMU page size + 0x20 bytes for the mapping header. */
    . = ALIGN(0x8000) + 0x20;
  } > default_rodata_seg
  .flash.appdesc : ALIGN(0x10)
  {
    /**

     * Mark flash.rodata start.

     * This can be used for mmu driver to maintain virtual address

     */
    _rodata_reserved_start = ABSOLUTE(.);
    _rodata_start = ABSOLUTE(.);
    /* !DO NOT PUT ANYTHING BEFORE THIS! */
    /* Should be the first.  App version info. */
    *(.rodata_desc .rodata_desc.*)
    /* Should be the second. Custom app version info. */
    *(.rodata_custom_desc .rodata_custom_desc.*)
    /**

     * Create an empty gap within this section. Thanks to this, the end of this

     * section will match .flash.rodata's begin address. Thus, both sections

     * will be merged when creating the final bin image.

     */
    . = ALIGN(ALIGNOF(.flash.rodata));
  } > default_rodata_seg
  ASSERT((ADDR(.flash.rodata) == ADDR(.flash.appdesc) + SIZEOF(.flash.appdesc)), "The gap between .flash.appdesc and .flash.rodata must not exist to produce the final bin image.")
  .flash.rodata : ALIGN(0x10)
  {
    _flash_rodata_start = ABSOLUTE(.);
    *(EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libclang_rt.builtins.a:_divsf3.* *libclang_rt.builtins.a:restore.* *libclang_rt.builtins.a:save.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:pmu_sleep.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:sleep_console.* *libesp_hw_support.a:systimer.* *libesp_mm.a:esp_cache.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_systimer.* *libesp_rom.a:esp_rom_tlsf.* *libesp_rom.a:esp_rom_wdt.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libgcc.a:save-restore.* *libhal.a:cache_hal.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:pmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_gpspi.* *libhal.a:spi_flash_hal_iram.* *libhal.a:spi_hal_iram.* *libhal.a:spi_slave_hal_iram.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .rodata EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libclang_rt.builtins.a:_divsf3.* *libclang_rt.builtins.a:restore.* *libclang_rt.builtins.a:save.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:pmu_sleep.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:sleep_console.* *libesp_hw_support.a:systimer.* *libesp_mm.a:esp_cache.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_systimer.* *libesp_rom.a:esp_rom_tlsf.* *libesp_rom.a:esp_rom_wdt.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libgcc.a:save-restore.* *libhal.a:cache_hal.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:pmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_gpspi.* *libhal.a:spi_flash_hal_iram.* *libhal.a:spi_hal_iram.* *libhal.a:spi_slave_hal_iram.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .rodata.* EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libclang_rt.builtins.a:_divsf3.* *libclang_rt.builtins.a:restore.* *libclang_rt.builtins.a:save.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:pmu_sleep.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:sleep_console.* *libesp_hw_support.a:systimer.* *libesp_mm.a:esp_cache.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_systimer.* *libesp_rom.a:esp_rom_tlsf.* *libesp_rom.a:esp_rom_wdt.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libgcc.a:save-restore.* *libhal.a:cache_hal.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:pmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_gpspi.* *libhal.a:spi_flash_hal_iram.* *libhal.a:spi_hal_iram.* *libhal.a:spi_slave_hal_iram.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .sdata2 EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libclang_rt.builtins.a:_divsf3.* *libclang_rt.builtins.a:restore.* *libclang_rt.builtins.a:save.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:pmu_sleep.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:sleep_console.* *libesp_hw_support.a:systimer.* *libesp_mm.a:esp_cache.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_systimer.* *libesp_rom.a:esp_rom_tlsf.* *libesp_rom.a:esp_rom_wdt.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libgcc.a:save-restore.* *libhal.a:cache_hal.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:pmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_gpspi.* *libhal.a:spi_flash_hal_iram.* *libhal.a:spi_hal_iram.* *libhal.a:spi_slave_hal_iram.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .sdata2.* EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libclang_rt.builtins.a:_divsf3.* *libclang_rt.builtins.a:restore.* *libclang_rt.builtins.a:save.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:pmu_sleep.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:sleep_console.* *libesp_hw_support.a:systimer.* *libesp_mm.a:esp_cache.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_systimer.* *libesp_rom.a:esp_rom_tlsf.* *libesp_rom.a:esp_rom_wdt.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libgcc.a:save-restore.* *libhal.a:cache_hal.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:pmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_gpspi.* *libhal.a:spi_flash_hal_iram.* *libhal.a:spi_hal_iram.* *libhal.a:spi_slave_hal_iram.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .srodata EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libclang_rt.builtins.a:_divsf3.* *libclang_rt.builtins.a:restore.* *libclang_rt.builtins.a:save.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:pmu_sleep.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:sleep_console.* *libesp_hw_support.a:systimer.* *libesp_mm.a:esp_cache.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_systimer.* *libesp_rom.a:esp_rom_tlsf.* *libesp_rom.a:esp_rom_wdt.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libgcc.a:_divsf3.* *libgcc.a:save-restore.* *libhal.a:cache_hal.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:pmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_gpspi.* *libhal.a:spi_flash_hal_iram.* *libhal.a:spi_hal_iram.* *libhal.a:spi_slave_hal_iram.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .srodata.*)
    *(.rodata_wlog_error .rodata_wlog_error.*)
    *(.rodata_wlog_info .rodata_wlog_info.*)
    *(.rodata_wlog_warning .rodata_wlog_warning.*)
    *(.irom1.text) /* catch stray ICACHE_RODATA_ATTR */
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    *(.gcc_except_table .gcc_except_table.*)
    *(.gnu.linkonce.e.*)
    /**

     * C++ constructor tables.

     *

     * Excluding crtbegin.o/crtend.o since IDF doesn't use the toolchain crt.

     *

     * RISC-V gcc is configured with --enable-initfini-array so it emits

     * .init_array section instead. But the init_priority sections will be

     * sorted for iteration in ascending order during startup.

     * The rest of the init_array sections is sorted for iteration in descending

     * order during startup, however. Hence a different section is generated for

     * the init_priority functions which is iterated in ascending order during

     * startup. The corresponding code can be found in startup.c.

     */
    
 . = ALIGN(4); 
 __init_priority_array_start = ABSOLUTE(.);
    KEEP (*(EXCLUDE_FILE (*crtend.* *crtbegin.*) .init_array.*))
    __init_priority_array_end = ABSOLUTE(.);
    
 . = ALIGN(4); 
 __init_array_start = ABSOLUTE(.);
    KEEP (*(EXCLUDE_FILE (*crtend.* *crtbegin.*) .init_array))
    __init_array_end = ABSOLUTE(.);
    /* Addresses of memory regions reserved via SOC_RESERVE_MEMORY_REGION() */
    
 . = ALIGN(4); 
 soc_reserved_memory_region_start = ABSOLUTE(.);
    KEEP (*(.reserved_memory_address))
    soc_reserved_memory_region_end = ABSOLUTE(.);
    /* System init functions registered via ESP_SYSTEM_INIT_FN */
    
 . = ALIGN(4); 
 _esp_system_init_fn_array_start = ABSOLUTE(.);
    KEEP (*(SORT_BY_INIT_PRIORITY(.esp_system_init_fn.*)))
    _esp_system_init_fn_array_end = ABSOLUTE(.);
    _rodata_end = ABSOLUTE(.);
    . = ALIGN(ALIGNOF(.eh_frame_hdr));
  } > default_rodata_seg
  ASSERT((ADDR(.eh_frame_hdr) == ADDR(.flash.rodata) + SIZEOF(.flash.rodata)), "The gap between .flash.rodata and .eh_frame_hdr must not exist to produce the final bin image.")
  .eh_frame_hdr :
  {
    . = ALIGN(ALIGNOF(.eh_frame));
  } > default_rodata_seg
  ASSERT((ADDR(.eh_frame) == ADDR(.eh_frame_hdr) + SIZEOF(.eh_frame_hdr)), "The gap between .eh_frame_hdr and .eh_frame must not exist to produce the final bin image.")
  .eh_frame :
  {
    . = ALIGN(ALIGNOF(.flash.tdata));
  } > default_rodata_seg
  ASSERT((ADDR(.flash.tdata) == ADDR(.eh_frame) + SIZEOF(.eh_frame)), "The gap between .eh_frame and .flash.tdata must not exist to produce the final bin image.")
  .flash.tdata :
  {
    _thread_local_data_start = ABSOLUTE(.);
    *(.tdata .tdata.* .gnu.linkonce.td.*)
    . = ALIGN(ALIGNOF(.flash.tbss));
    _thread_local_data_end = ABSOLUTE(.);
  } > default_rodata_seg
  ASSERT((ADDR(.flash.tbss) == ADDR(.flash.tdata) + SIZEOF(.flash.tdata)), "The gap between .flash.tdata and .flash.tbss must not exist to produce the final bin image.")
  .flash.tbss (NOLOAD) :
  {
    _thread_local_bss_start = ABSOLUTE(.);
    *(.tbss .tbss.* .gnu.linkonce.tb.*)
    *(.tcommon .tcommon.*)
    _thread_local_bss_end = ABSOLUTE(.);
  } > default_rodata_seg
  /**

   * This section contains all the rodata that is not used

   * at runtime, helping to avoid an increase in binary size.

   */
  .flash.rodata_noload (NOLOAD) :
  {
    /**

     * This symbol marks the end of flash.rodata. It can be utilized by the MMU

     * driver to maintain the virtual address.

     * NOLOAD rodata may not be included in this section.

     */
    _rodata_reserved_end = ADDR(.flash.tbss);
    *(.rodata_wlog_debug .rodata_wlog_debug.*)
    *(.rodata_wlog_verbose .rodata_wlog_verbose.*)
  } > default_rodata_seg
  /* Marks the end of data, bss and possibly rodata */
  .dram0.heap_start (NOLOAD) :
  {
    
 . = ALIGN(16); 
 _heap_start = ABSOLUTE(.);
  } > sram_seg
  /* DWARF 1 */
  .debug 0 : { *(.debug) }
  .line 0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo 0 : { *(.debug_srcinfo) }
  .debug_sfnames 0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges 0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info 0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev 0 : { *(.debug_abbrev) }
  .debug_line 0 : { *(.debug_line) }
  .debug_frame 0 : { *(.debug_frame) }
  .debug_str 0 : { *(.debug_str) }
  .debug_loc 0 : { *(.debug_loc) }
  .debug_macinfo 0 : { *(.debug_macinfo) }
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  /* DWARF 3 */
  .debug_ranges 0 : { *(.debug_ranges) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames 0 : { *(.debug_varnames) }
  /* GNU DWARF 2 extensions */
  .debug_gnu_pubnames 0 : { *(.debug_gnu_pubnames) }
  .debug_gnu_pubtypes 0 : { *(.debug_gnu_pubtypes) }
  /* DWARF 4 */
  .debug_types 0 : { *(.debug_types) }
  /* DWARF 5 */
  .debug_addr 0 : { *(.debug_addr) }
  .debug_line_str 0 : { *(.debug_line_str) }
  .debug_loclists 0 : { *(.debug_loclists) }
  .debug_macro 0 : { *(.debug_macro) }
  .debug_names 0 : { *(.debug_names) }
  .debug_rnglists 0 : { *(.debug_rnglists) }
  .debug_str_offsets 0 : { *(.debug_str_offsets) }
  .comment 0 : { *(.comment) }
  .note.GNU-stack 0: { *(.note.GNU-stack) }
  .riscv.attributes 0: { *(.riscv.attributes) }
  /DISCARD/ :
  {
   /**

    * Discarding .rela.* sections results in the following mapping:

    * .rela.text.* -> .text.*

    * .rela.data.* -> .data.*

    * And so forth...

    */
   *(.rela.*)
   *(.eh_frame_hdr)
   *(.eh_frame)
  }
}
